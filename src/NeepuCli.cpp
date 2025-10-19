#include <ctime>
#include <iostream>
#include <new>
#include <string>
#include "3rd/cxxopts.h"
#include "3rd/json.hpp"
#include "3rd/AsulFormatString.h"
#include "3rd/httplib.h"
#include "3rd/VDXSingle.h"
#include "3rd/FileTools.h"
#include "3rd/NEEPU_SDK.h"

#define DATA_JSON "data.json"
#define VERSION "alpha-v0.0.1"

using namespace AsulKit::FileTools;
using namespace NEEPULoginSDK;
using std::cout,std::endl,std::cerr;

std::string getRandomStr(){
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    srand(time(NULL));
    for (size_t i = 0; i < 4; ++i) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

int main(int argc, char** argv) {
    asul_formatter().installColorFormatAdapter();
    asul_formatter().installLogLabelAdapter();
    asul_formatter().installAskLabelAdapter();

    /*
        req_json["user"] = req.user;
        req_json["pass"] = req.pass;
        req_json["code"] = req.code;
        req_json["authmode"] = req.authmode;
        req_json["pool"] = req.pool;
        req_json["isp_id"] = req.isp_id;
        req_json["pxyacct"] = req.pxyacct;
    */

    std::string account="",
                password="",
                smsCode="",
                pool="",
                pxyacct="";
        int     authmode=0,
                isp_id=0;

    if(argc==1 && fileExist(DATA_JSON)){
        std::ifstream inFile(DATA_JSON);
        nlohmann::json jsonData;
        inFile >> jsonData;
        try{
            account = jsonData["user"];
            password = jsonData["pass"];
            smsCode = jsonData["code"];
            authmode = jsonData["authmode"];
            pool = jsonData["pool"];
            isp_id = jsonData["isp_id"];
            pxyacct = jsonData["pxyacct"];
        }catch(...){
            std::cerr << f("(ERROR) 读取配置文件失败\n");
            return Err;
        }
        
    }else{
        VDX * Crypto = new VDX("5a3b9f207411a8ed");
        try {
            cxxopts::Options options("NeepuCLI", "登陆到 NEEPU_STU -- CLI Version");
            options.add_options()
                ("v,version","显示版本信息")
                ("h,help", "显示帮助信息")
                ("a,account", "输入账户名",cxxopts::value<std::string>())
                ("p,password", "输入密码", cxxopts::value<std::string>())
                ("l,pool", f("输入运营商 [{GREEN}:{DARK_GRAY},{RED}:{DARK_GRAY}]","联通","liantong","移动","yidong"), cxxopts::value<std::string>())
                ;
            auto result = options.parse(argc, argv);
            if(result.count("version")){
                cout << f("{YELLOW} - {LIGHT_MAGENTA}\n{LIGHT_GRAY}: {DARK_GRAY}",argv[0],VERSION,"编译时间",__DATE__);
                return Accepted;
            }
            if(result.count("help")){
                options.show_positional_help();
                return Accepted;
            }
            if(result.count("account")) {
                account = result["account"].as<std::string>();
            }
            if(result.count("password")) {
                password = result["password"].as<std::string>();
            }
            if(result.count("pool")){
                pool = result["pool"].as<std::string>();
            }
        } catch (const std::exception& e) {
            std::cout << f("(ERROR) 解析命令行参数时出错:{LIGHT_GRAY} \n",e.what());
            return Err;
        }

        if (account.empty() || password.empty()) {
            std::cout << f("(ERROR) 账户或密码不能为空\n");
            return Err;
        }
        if (pool.empty()){
            std::cout << f("(ERROR) 服务提供运营商不能为空\n");
            return Err;
        }
        std::string rdStr=getRandomStr();
        // std::string rdStr="abcd";
        std::cout << f("随机数：{}\n账户: {}\n密码:\n\t原始:{}\n\t加密:{}\n",rdStr, account,password, VDX::to_hex(Crypto->encrypt(rdStr+password)));
        delete Crypto;
    }
    
    
	LoginRequest req;
	req.user = account;
	req.pass = password;
	req.code = smsCode;
	req.authmode = authmode;
	req.pool = pool;
	req.isp_id = isp_id;
	req.pxyacct = pxyacct;

	LoginResponse resp = sendLoginRequest("http://202.198.8.232/api/login.php", req);
	LoginResponse resp_= sendGetStatusRequest("http://202.198.8.232/api/stat.php", req);

	if (resp.success) {
        cout << f("(SUCCESS) 提交报表成功！返回码：{GREEN}\n(INFO)消息：{DARK_GRAY}\n", resp.retCode, resp.msg);
        cout << f("(INFO) 附加数据：{DARK_GRAY}\n", resp.data.dump(4));
	} else {
        cerr << f("(ERROR) 提交报表失败！错误码：{RED}\n",resp.retCode);
        cerr << f("(INFO) 错误信息：{DARK_GRAY}\n", resp.msg);
	}
	if (resp_.success) {
        cout << f("(SUCCESS) 登录成功！返回码：{GREEN}\n(INFO)消息：{DARK_GRAY}\n", resp_.retCode, resp_.msg);
        cout << f("(INFO) 附加数据：{DARK_GRAY}\n", resp_.data.dump(4));
        auto ret=writeToFile(DATA_JSON,LoginRequest2Json(req).dump(4));
        if(ret.first){
            cout << f("(SUCCESS) 配置文件成功保存至：{DARK_GRAY}\n",DATA_JSON);
        }
    } else {
        cerr << f("(ERROR) 登录失败！错误码：{RED}\n(INFO)错误信息：{DARK_GRAY}\n", resp_.retCode, resp_.msg);
        cerr << f("(INFO) 附加数据：{DARK_GRAY}\n", resp_.data.dump(4));
	}

    return Accepted;
}
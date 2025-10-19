module("luci.controller.neepu-login-tool.neepu-login-tool", package.seeall)

function index()
    entry({"admin", "services", "neepu-login-tool"}, cbi("neepu-login-tool/neepu-login-tool"), _("NEEPU Login Tool"), 60)
end

function submit()
    local account = luci.http.formvalue("account")
    local password = luci.http.formvalue("password")
    local isp = luci.http.formvalue("isp")
    
    local isp_param = isp == "联通" and "liantong" or "yidong"
    
    local command = string.format("./neepu-login-tool --account %s --password %s --pool %s", account, password, isp_param)
    os.execute(command)
    
    luci.http.redirect(luci.dispatcher.build_url("admin/services/neepu-login-tool"))
end
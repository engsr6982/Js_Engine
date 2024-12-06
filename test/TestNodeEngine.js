// TODO: 修正 __dirname 和 __filename 路径问题

const JSEAPI = loadNativeType("JSEAPI"); // class

JSEAPI.register_plugin({
  name: "TestNodeEngine", // string
  version: "v0.1.0", // string
  description: "测试嵌入式Node.js引擎", // string
  load: "PostWorld", // PostWorld / Startup
  authors: ["engsr6982"], // vector<string>
  contributors: ["engsr6982"], // vector<string>
  website: "https://github.com/engsr6982/Js_Engine", // string
  prefix: "NodePlugin", // string
  provides: [], // vector<string>
  depend: [], // vector<string>
  soft_depend: [], // vector<string>
  load_before: [], // vector<string>
  default_permission: "Operator", // True / False / Operator / NotOperator
  commands: [], // vector<Command>
  permissions: [], // vector<Permission>

  onLoad: function () {},
  onEnable: function () {},
  onDisable: function () {},
});

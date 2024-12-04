// 已知问题:
// Worker Thread 无法绑定 API  [无法解决]
// __dirname 和 __filename 损坏 [TODO]

// 加载底层类型，脚本层 new 测试
// const TestClass = loadCppType("TestClass");

// TestClass.Print("hello world");
// let obj = new TestClass(123);

// TestClass.Print(obj.X);
// obj.X = 99;
// TestClass.Print(obj.X);

// TestClass.Print("ret = " + obj.Add(1, 3));

import { bFunc } from "./b.js";
console.warn(bFunc());

// 从底层获取已有类实例，脚本层使用
const inst = getNativeTestClass();
console.log(inst.X);
console.log(inst.Add(114514, 1));

const TestB = loadCppType("TestB");
const b = new TestB(inst);

TestB.Print(b.p.X); // x = (TestClass*).x
inst.X = 999;

const inst1 = b.GetP();
console.log(inst1.X);

// 事件循环测试
// setInterval(() => {
//   const t = new Date();
//   console.log(
//     `[${t.getMinutes()}:${t.getSeconds()}:${t.getMilliseconds()}] Main Thread: X = ${
//       inst.X
//     }`
//   );
// }, 1000);

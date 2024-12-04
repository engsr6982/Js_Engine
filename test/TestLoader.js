// 测试worker
const {
  Worker,
  isMainThread,
  parentPort,
  workerData,
} = require("node:worker_threads");

if (isMainThread) {
  // 加载底层类型，脚本层 new 测试
  const TestClass = loadCppType("TestClass");

  TestClass.Print("hello world");
  let obj = new TestClass(123);

  TestClass.Print(obj.X);
  obj.X = 99;
  TestClass.Print(obj.X);

  TestClass.Print("ret = " + obj.Add(1, 3));

  // 从底层获取已有类实例，脚本层使用
  const inst = getNativeTestClass();
  console.log(inst.X);
  console.log(inst.Add(114514, 1));

  // 测试worker
  // 主线程代码
  TestClass.Print("Main Thread: Starting worker...");

  setInterval(() => {
    const t = new Date();
    console.log(
      `[${t.getMinutes()}:${t.getSeconds()}:${t.getMilliseconds()}] Main Thread: X = ${
        inst.X
      }`
    );
  }, 1000);

  const worker = new Worker("./plugins/test/TestLoader.js", {});
  worker.on("message", (msg) => {
    console.log(`Worker said: ${msg}`);
  });
  worker.on("error", (err) => {
    console.error(err);
  });
  worker.on("exit", (code) => {
    console.log(`Worker stopped with exit code ${code}`);
  });
} else {
  // Worker线程代码
  try {
    try {
      // 尝试获取底层实例
      const inst = getNativeTestClass();
      parentPort.postMessage(
        `Worker Thread: Got native instance, X = ${inst.X}`
      );
      parentPort.postMessage(
        `Worker Thread: Add result = ${inst.Add(114514, 1)}`
      );
    } catch (e) {
      parentPort.postMessage(
        `Worker Thread: Error getting native instance: ${e}`
      );
    }

    // 向主线程发送消息
    parentPort.postMessage("Hello from worker!");

    // 接收主线程消息
    parentPort.on("message", (msg) => {
      parentPort.postMessage(`Worker Thread received: ${JSON.stringify(msg)}`);
    });
  } catch (e) {
    if (parentPort) {
      parentPort.postMessage({ error: e.message });
    }
  }
}

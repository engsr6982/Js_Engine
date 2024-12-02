// "use strict";
/// <reference path="../types/index.d.ts" />

const logger = JSE_Logger;
JSE_EndStone.register_plugin({
  name: "TestLoader",
  version: "1.0.0",
  description: "TestLoader description",

  onLoad: () => {
    logger.warn("TestLoader loaded");
  },
  onEnable: () => {
    logger.warn("TestLoader enabled");

    test_plugin_api();
  },
  onDisable: () => {
    logger.warn("TestLoader disabled");
  },
});

function test_plugin_api() {
  logger.warn("==== Test Plugin API  ====");
  const pl = JSE_EndStone.get_plugin();

  logger.warn("api_version: ", pl.api_version);
}

declare interface JSE_Plugin_Register {
  name: string;
  version: string;
  description: string;

  onLoad(): void;
  onEnable(): void;
  onDisable(): void;
}

declare class JSE_EndStone {
  static register_plugin(information: JSE_Plugin_Register): void;

  static get_plugin(): any; // TODO: Define plugin type
}

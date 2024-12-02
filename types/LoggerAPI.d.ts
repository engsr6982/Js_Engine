// endstone logger.h
declare enum LoggerLevel {
  Trace = 0,
  Debug = 1,
  Info = 2,
  Warning = 3,
  Error = 4,
  Critical = 5,
  Off = 6,
}

declare type Color =
  | /**深蓝色*/ "dk_blue"
  | /**深绿色*/ "dk_green"
  | /**浅蓝色*/ "bt_blue"
  | /**深红色*/ "dk_red"
  | /**紫色*/ "purple"
  | /**深黄色*/ "dk_yellow"
  | /**灰色*/ "grey"
  | /**天蓝色*/ "sky_blue"
  | /**蓝色*/ "blue"
  | /**绿色*/ "green"
  | /**青色*/ "cyan"
  | /**红色*/ "red"
  | /**粉色*/ "pink"
  | /**黄色*/ "yellow"
  | /**白色*/ "white";

declare class JSE_Logger {
  static log(level: LoggerLevel, ...message: any[]): boolean;
  static info(...message: any[]): boolean;
  static warn(...message: any[]): boolean;
  static error(...message: any[]): boolean;
  static debug(...message: any[]): boolean;

  static color_log(color: Color, ...message: any[]): boolean;
  static format(format_str: string, ...args: any[]): string;
}

import { Module, Global, Provider } from '@nestjs/common';

let webUiConfigPath: string = null;
let webServerConfigPath: string = null;
let webUiBuildPath: string = null;
let webServerConfigDirectoryPath: string = null;
let webUiConfigDirectoryPath: string = null;

const WebUiConfigPathProvider: Provider = {
  provide: 'WEB_UI_CONFIG_PATH',
  useFactory: () => webUiConfigPath,
};

const WebServerConfigPathProvider: Provider = {
  provide: 'WEB_SERVER_CONFIG_PATH',
  useFactory: () => webServerConfigPath,
};

const WebUiBuildPathProvider: Provider = {
  provide: 'WEB_UI_BUILD_PATH',
  useFactory: () => webUiBuildPath,
};

const WebServerConfigDirectoryPathProvider: Provider = {
  provide: 'WEB_SERVER_CONFIG_DIRECTORY_PATH',
  useFactory: () => webServerConfigDirectoryPath,
};

const WebUiConfigDirectoryPathProvider: Provider = {
  provide: 'WEB_UI_CONFIG_DIRECTORY_PATH',
  useFactory: () => webUiConfigDirectoryPath,
};

@Global()
@Module({
  providers: [WebUiConfigPathProvider, WebServerConfigPathProvider, WebUiBuildPathProvider, WebServerConfigDirectoryPathProvider, WebUiConfigDirectoryPathProvider],
  exports: [WebUiConfigPathProvider, WebServerConfigPathProvider, WebUiBuildPathProvider, WebServerConfigDirectoryPathProvider, WebUiConfigDirectoryPathProvider],
})
export class ConfigModule {
  static setWebUiConfigPath(value: string) {
    webUiConfigDirectoryPath = value;
    webUiConfigPath = `${value}/web_ui.json`;
  }

  static setWebServerConfigPath(value: string) {
    webServerConfigDirectoryPath = value;
    webServerConfigPath = `${value}/web_server.json`;
  }

  static setWebUiBuildPath(value: string) {
    webUiBuildPath = value;
  }
}

import { CreateAppSettingsDto } from '../dto/create-appSettings.dto'
import { RadiusTestDto } from '../dto/radiusTest-appSettings.dto'
import { AppSetting } from './appSetting.interface'

export const APP_SETTINGS_SERVICE = 'AppSettingsService'

export interface IAppSettingsService {
    create(createAppSettingsDto: CreateAppSettingsDto): Promise<AppSetting>
    find(): Promise<AppSetting>
    radiusTest(radiusSettings: RadiusTestDto): Promise<string>
}

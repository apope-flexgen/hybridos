import { Injectable } from '@nestjs/common'
import { InjectModel } from '@nestjs/mongoose'
import { Model } from 'mongoose'

import { AuthRadius } from '../radius/authRadius'
import { CreateAppSettingsDto } from './dto/create-appSettings.dto'
import { RadiusTestDto } from './dto/radiusTest-appSettings.dto'
import { AppSetting } from './interfaces/appSetting.interface'
import { AppSettingDocument } from './schemas/appSetting.schema'

@Injectable()
export class AppSettingsService {
    constructor(
        @InjectModel('appsettings')
        private readonly appSettingsModel: Model<AppSettingDocument>
    ) {}
    async create(createAppSettingsDto: CreateAppSettingsDto): Promise<AppSetting> {
        const createdAppSettings = await this.appSettingsModel.create(createAppSettingsDto)
        return createdAppSettings
    }
    async find(): Promise<AppSetting> {
        const site = await this.appSettingsModel.findOne({}).exec()

        // if no appSetting is found,
        //    create -> save -> return a new appSetting with default values
        if (!site) {
            const defaultAppSettings = await this.appSettingsModel.create({})
            return defaultAppSettings
        }
        return site
    }
    async radiusTest(radiusSettings: RadiusTestDto): Promise<string> {
        const authRadius = new AuthRadius(
            radiusSettings.ip_address,
            radiusSettings.secret_phrase,
            radiusSettings.port,
            radiusSettings.wait_time
        )

        return new Promise((resolve) => {
            authRadius
                .authenticate(radiusSettings.username, radiusSettings.password)
                .onAccept((decodedPacket) => {
                    const role = authRadius.getAttributeRole(decodedPacket)
                    resolve(role)
                })
                .onReject(() => {
                    resolve(null)
                })
                .onError(() => {
                    resolve(null)
                })
                .onTimeout(() => {
                    resolve(null)
                })
        })
    }
}

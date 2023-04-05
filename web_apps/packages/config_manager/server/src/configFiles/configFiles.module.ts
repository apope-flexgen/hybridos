import { forwardRef, Module } from '@nestjs/common'
import { SequelizeModule } from '@nestjs/sequelize'
import { ConfigPullerService } from './configPuller.service'
import { ConfigLocation } from './models/configLocation.model'
import { ConfigLocationType } from './models/configLocationType.model'
import { ConfigVersion } from './models/configVersion.model'
import { Controller } from './models/controller.model'
import { ControllerType } from './models/controllerType.model'
import { ControllerVersion } from './models/controllerVersion.model'
import { Customer } from './models/customer.model'
import { HybridOSVersion } from './models/hybridOSVersion.model'
import { ModuleService } from './models/module.model'
import { ModuleConfigFile } from './models/moduleConfigFile.model'
import { ModuleType } from './models/moduleType.model'
import { Site } from './models/site.model'
import { MongooseModule } from '@nestjs/mongoose'
import { ConfigFile, ConfigFileSchema } from './schemas/configFile.schema'
import { UsersModule } from '../users/users.module'
import { LifecycleStatus } from './models/lifecycle.model'
import { ConfigHistoryService } from './configHistory.service'
import { ConfigFileController } from './configFile.controller'
import { ConfigFileService } from './configFile.service'
import { ModuleControllerVersion } from './models/moduleControllerVersion.model'
import { ConfigHelperService } from './configHelper.service'

@Module({
    imports: [
        SequelizeModule.forFeature([
            ConfigLocation,
            ConfigLocationType,
            ConfigVersion,
            Controller,
            ControllerType,
            ControllerVersion,
            Customer,
            HybridOSVersion,
            ModuleConfigFile,
            ModuleType,
            Site,
            ModuleService,
            LifecycleStatus,
            ModuleControllerVersion,
        ]),
        MongooseModule.forFeature([{ name: ConfigFile.name, schema: ConfigFileSchema }]),
        UsersModule,
    ],
    providers: [ConfigPullerService, ConfigHistoryService, ConfigFileService, ConfigHelperService],
    controllers: [ConfigFileController],
})
export class ConfigFilesModule {}

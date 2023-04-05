/* eslint-disable max-lines */
/* eslint-disable max-statements */
import { forwardRef, Inject, Injectable, OnApplicationBootstrap } from '@nestjs/common'
import { InjectModel } from '@nestjs/sequelize'

import * as fs from 'fs'
import * as glob from 'glob'
import { Sequelize } from 'sequelize-typescript'
import { Customer } from './models/customer.model'
import { Site } from './models/site.model'
import { Controller } from './models/controller.model'
import { ControllerType } from './models/controllerType.model'
import { HybridOSVersion } from './models/hybridOSVersion.model'
import { ControllerVersion } from './models/controllerVersion.model'
import { ConfigVersion } from './models/configVersion.model'
import { ModuleService } from './models/module.model'
import { ModuleType } from './models/moduleType.model'
import { ModuleConfigFile } from './models/moduleConfigFile.model'
import { ConfigLocation } from './models/configLocation.model'
import { ConfigLocationType } from './models/configLocationType.model'
import { LifecycleStatus } from './models/lifecycle.model'
import { ConfigFile, ConfigFileDocument } from './schemas/configFile.schema'
import { Model } from 'mongoose'
import { InjectModel as InjectMongooseModel } from '@nestjs/mongoose'
import { ModuleControllerVersion } from './models/moduleControllerVersion.model'
import { ConfigFileService } from './configFile.service'
import { ConfigHelperService } from './configHelper.service'
import { UsersService } from '../users/users.service'

@Injectable()
export class ConfigPullerService implements OnApplicationBootstrap {
    constructor(
        @InjectModel(Customer) private customerRepository: typeof Customer,
        @InjectModel(Site) private siteRepository: typeof Site,
        @InjectModel(Controller) private controllerRepository: typeof Controller,
        @InjectModel(ControllerType) private controllerTypeRepository: typeof ControllerType,
        @InjectModel(HybridOSVersion) private hybridOSVersionRepository: typeof HybridOSVersion,
        @InjectModel(ControllerVersion)
        private controllerVersionRepository: typeof ControllerVersion,
        @InjectModel(ConfigVersion) private configVersionRepository: typeof ConfigVersion,
        @InjectModel(ModuleService) private moduleServiceRepository: typeof ModuleService,
        @InjectModel(ModuleType) private moduleTypeRepository: typeof ModuleType,
        @InjectModel(ModuleConfigFile) private moduleConfigFileRepository: typeof ModuleConfigFile,
        @InjectModel(ConfigLocation) private configLocationRepository: typeof ConfigLocation,
        @InjectModel(ConfigLocationType)
        private configLocationTypeRepository: typeof ConfigLocationType,
        @InjectModel(LifecycleStatus) private lifecycleStatusRepository: typeof LifecycleStatus,
        @InjectModel(ModuleControllerVersion)
        private moduleControllerVerionRepository: typeof ModuleControllerVersion,

        private sequelize: Sequelize,

        @InjectMongooseModel(ConfigFile.name)
        private readonly configFileModel: Model<ConfigFileDocument>,

        private readonly usersService: UsersService,
        private readonly configHelperService: ConfigHelperService
    ) {}
async onApplicationBootstrap() {
        const start = Date.now()

        // TODO: make this filepath configurable
        const filePath = './src/configFiles/exampleConfigSets/oneControllerConfigs'

        // pull configs from file system
        // const numFiles = await this.pullConfigsFromFileSystem(filePath)

        // pull configs from mongo
        // initialize sqlite db with data to find documents in mongo
        await this.initializeDatabases(filePath)
        const numFiles = await this.pullConfigsFromMongo()

        const end = Date.now()

        console.log(`${numFiles} configFiles saved to the database`)
        console.log('time elapsed: ', end - start, 'ms')
    }
// this function is used to initialize the databases before pulling configs from mongo
    initializeDatabases = async (directory: string) => {
        const filePaths = this.configHelperService.getJSONFiles(directory)

        // clear mongo db
        await this.configFileModel.deleteMany({})

        const fileInfos = filePaths.map((filePath) => {
            const pieces = filePath.split('/').reverse()

            const fileContents = fs.readFileSync(filePath, { encoding: 'utf-8' })

            return {
                path: filePath,
                name: pieces[0],
                module: pieces[1],
                controller: pieces[3],
                fileContents,
            }
        })

        const transaction = await this.sequelize.transaction()
        try {
            for (const fileInfo of fileInfos) {
                // populate mongo db with config file contents
                const created = await this.configFileModel.create({
                    fileContents: Buffer.from(fileInfo.fileContents),
                    name: fileInfo.name,
                })
                // save the mongoId to be stored in the ConfigFileLocation sqlite repo
                const mongoId = created._id.toString()

                // customer
                const customer = await this.configHelperService.findOrCreate(
                    this.customerRepository,
                    { name: '?BRP' },
                    transaction
                )
                // site
                const site = await this.configHelperService.findOrCreate(
                    this.siteRepository,
                    { name: '?Batcave', fk_customer_id: customer.id },
                    transaction
                )
                // controller type
                const controllerType = await this.configHelperService.findOrCreate(
                    this.controllerTypeRepository,
                    { name: fileInfo.controller },
                    transaction
                )

                // hybridos version
                const hybridOSVersion = await this.configHelperService.findOrCreate(
                    this.hybridOSVersionRepository,
                    { name: '?v11.1' },
                    transaction
                )
                // controller
                const controller = await this.configHelperService.findOrCreate(
                    this.controllerRepository,
                    {
                        name: fileInfo.controller,
                        fk_site_id: site.id,
                        fk_controllerType_id: controllerType.id,
                        fk_hybridOSVersion_id: hybridOSVersion.id,
                    },
                    transaction
                )
                // config version
                const configVersion = await this.configHelperService.findOrCreate(
                    this.configVersionRepository,
                    { name: '?v0.0.1' },
                    transaction
                )

                const uploader = await this.usersService.findOrCreate(
                    { username: 'SystemUser' },
                    transaction
                )

                const lifecycle = await this.configHelperService.findOrCreate(
                    this.lifecycleStatusRepository,
                    { name: 'uploaded' },
                    transaction
                )

                // controller version
                const controllerVersion = await this.configHelperService.findOrCreate(
                    this.controllerVersionRepository,
                    {
                        version: 1,
                        fk_controller_id: controller.id,
                        fk_configVersion_id: configVersion.id,
                        fk_uploader_id: uploader.id,
                        fk_lifecycle_id: lifecycle.id,
                    },
                    transaction
                )

                // module type
                const moduleType = await this.configHelperService.findOrCreate(
                    this.moduleTypeRepository,
                    { name: fileInfo.module },
                    transaction
                )

                // config location type
                const configLocationType = await this.configHelperService.findOrCreate(
                    this.configLocationTypeRepository,
                    { name: 'mongo' },
                    transaction
                )

                // module service
                const moduleService = await this.configHelperService.findOrCreate(
                    this.moduleServiceRepository,
                    {
                        name: fileInfo.module,
                        // fk_controllerVersion_id: controllerVersion.id,
                        fk_moduletype_id: moduleType.id,
                        // fk_location_id: configLocation.id,
                    },
                    transaction
                )

                // config location
                const configLocation = await this.configHelperService.findOrCreate(
                    this.configLocationRepository,
                    {
                        name: mongoId,
                        fk_module_id: moduleService.id,
                        fk_configLocationType_id: configLocationType.id,
                    },
                    transaction
                )

                const moduleControllerVersion = await this.configHelperService.findOrCreate(
                    this.moduleControllerVerionRepository,
                    {
                        controller_version_id: controllerVersion.id,
                        module_id: moduleService.id,
                    },
                    transaction
                )
            }

            // complete transaction
            await transaction.commit()
            return filePaths.length
        } catch (error) {
            await transaction.rollback()
            throw new Error(error)
        }
    }
pullConfigsFromMongo = async (): Promise<number> => {
        // get list of mongoIDs stored in the sql database
        const locations = await this.configLocationRepository.findAll({
            attributes: ['name', 'fk_module_id'],
            include: [{ model: ConfigLocationType, where: { name: 'mongo' }, attributes: [] }],
        })

        const transaction = await this.sequelize.transaction()
        try {
            const uploader = await this.usersService.findOrCreate(
                { username: 'SystemUser' },
                transaction
            )

            const lifecycle = await this.configHelperService.findOrCreate(
                this.lifecycleStatusRepository,
                { name: 'uploaded' },
                transaction
            )

            for (const location of locations) {
                const mongoFile = await this.configFileModel.findById(location.name)

                const configFile = await this.moduleConfigFileRepository.create(
                    {
                        name: mongoFile.name,
                        file: await this.configHelperService.processFileContents(
                            mongoFile.fileContents
                        ),
                        version: 1,
                        fk_module_id: location.fk_module_id,
                        fk_uploader_id: uploader.id,
                        fk_lifecycle_id: lifecycle.id,
                    },
                    { transaction: transaction }
                )
            }

            // complete transaction
            await transaction.commit()
            return locations.length
        } catch (error) {
            await transaction.rollback()
            throw new Error(error)
        }
    }
pullConfigsFromFileSystem = async (directory: string): Promise<number> => {
        // collect filepaths of configFiles
        let filePaths: string[] = []
        try {
            filePaths = this.configHelperService.getJSONFiles(directory)
        } catch (error) {
            // if an error occurs while reading files from given directory, throw an error
            console.error('error getting file paths')
            throw error
        }

        // if directory exists but no json files are present, exit
        if (filePaths.length === 0) {
            console.error('no config files exist at location')
            return 0
        }

        // read files from file location and save them to the database along with new version
        const numFiles = await this.saveFilesToDatabase(filePaths)

        // await this.printInsertedData()
        return numFiles
    }
saveFilesToDatabase = async (filePaths: string[]): Promise<number> => {
        const fileInfos = filePaths.map((filePath) => {
            const pieces = filePath.split('/').reverse()

            const fileContents = fs.readFileSync(filePath, { encoding: 'utf-8' })

            return {
                path: filePath,
                name: pieces[0],
                module: pieces[1],
                controller: pieces[3],
                fileContents,
            }
        })

        const transaction = await this.sequelize.transaction()
        try {
            for (const fileInfo of fileInfos) {
                // customer
                const customer = await this.configHelperService.findOrCreate(
                    this.customerRepository,
                    { name: '?BRP' },
                    transaction
                )
                // site
                const site = await this.configHelperService.findOrCreate(
                    this.siteRepository,
                    { name: '?Batcave', fk_customer_id: customer.id },
                    transaction
                )
                // controller type
                const controllerType = await this.configHelperService.findOrCreate(
                    this.controllerTypeRepository,
                    { name: fileInfo.controller },
                    transaction
                )

                // hybridos version
                const hybridOSVersion = await this.configHelperService.findOrCreate(
                    this.hybridOSVersionRepository,
                    { name: '?v11.1' },
                    transaction
                )
                // controller
                const controller = await this.configHelperService.findOrCreate(
                    this.controllerRepository,
                    {
                        name: fileInfo.controller,
                        fk_site_id: site.id,
                        fk_controllerType_id: controllerType.id,
                        fk_hybridOSVersion_id: hybridOSVersion.id,
                    },
                    transaction
                )
                // config version
                const configVersion = await this.configHelperService.findOrCreate(
                    this.configVersionRepository,
                    { name: '?v0.0.1' },
                    transaction
                )

                const uploader = await this.usersService.findOrCreate(
                    {
                        username: 'SystemUser',
                    },
                    transaction
                )

                const lifecycle = await this.configHelperService.findOrCreate(
                    this.lifecycleStatusRepository,
                    {
                        name: 'uploaded',
                    },
                    transaction
                )

                // controller version
                const controllerVersion = await this.configHelperService.findOrCreate(
                    this.controllerVersionRepository,
                    {
                        version: 1,
                        fk_controller_id: controller.id,
                        fk_configVersion_id: configVersion.id,
                        fk_uploader_id: uploader.id,
                        fk_lifecycle_id: lifecycle.id,
                    },
                    transaction
                )

                // module type
                const moduleType = await this.configHelperService.findOrCreate(
                    this.moduleTypeRepository,
                    { name: fileInfo.module },
                    transaction
                )

                // config location type
                const configLocationType = await this.configHelperService.findOrCreate(
                    this.configLocationTypeRepository,
                    { name: 'file system' },
                    transaction
                )

                // module service
                const moduleService = await this.configHelperService.findOrCreate(
                    this.moduleServiceRepository,
                    {
                        name: fileInfo.module,
                        fk_moduletype_id: moduleType.id,
                    },
                    transaction
                )

                // config location
                const configLocation = await this.configHelperService.findOrCreate(
                    this.configLocationRepository,
                    {
                        name: fileInfo.path,
                        fk_module_id: moduleService.id,
                        fk_configLocationType_id: configLocationType.id,
                    },
                    transaction
                )

                const moduleControllerVersion = await this.configHelperService.findOrCreate(
                    this.moduleControllerVerionRepository,
                    {
                        controller_version_id: controllerVersion.id,
                        module_id: moduleService.id,
                    },
                    transaction
                )

                // module config file
                const moduleConfigFile = await this.configHelperService.findOrCreate(
                    this.moduleConfigFileRepository,
                    {
                        name: fileInfo.name,
                        file: await this.configHelperService.processFile(fileInfo.path),
                        version: 1,
                        fk_module_id: moduleService.id,
                        fk_uploader_id: uploader.id,
                        fk_lifecycle_id: lifecycle.id,
                    },

                    transaction
                )
            }

            // complete transaction
            await transaction.commit()
            return filePaths.length
        } catch (error) {
            await transaction.rollback()
            throw new Error(error)
        }
    }
// TODO: change to log to file rather than console
    printInsertedData = async () => {
        console.log('\n\ninserted data: \n\n\n')

        // await this.configHelperService.printRepo(this.customerRepository)
        // await this.configHelperService.printRepo(this.siteRepository)
        // await this.configHelperService.printRepo(this.controllerRepository)
        // await this.configHelperService.printRepo(this.controllerTypeRepository)
        // await this.configHelperService.printRepo(this.hybridOSVersionRepository)
        await this.configHelperService.printRepo(this.controllerVersionRepository)
        // await this.configHelperService.printRepo(this.configVersionRepository)
        // await this.configHelperService.printRepo(this.moduleServiceRepository)
        // await this.configHelperService.printRepo(this.moduleTypeRepository)
        // await this.configHelperService.printRepo(this.configLocationRepository)
        // await this.configHelperService.printRepo(this.configLocationTypeRepository)
        await this.configHelperService.printRepo(this.moduleConfigFileRepository)
        // await this.configHelperService.printRepo(this.uploaderRepository)
        // await this.configHelperService.printRepo(this.lifecycleStatusRepository)
    }
}

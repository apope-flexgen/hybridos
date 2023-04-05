/* eslint-disable max-lines */
/* eslint-disable max-nested-callbacks */
import { getModelToken } from '@nestjs/sequelize'
import { Test } from '@nestjs/testing'
import { getAttributes, Sequelize } from 'sequelize-typescript'
import { ConfigPullerService } from '../configPuller.service'

import * as fs from 'fs'
import { ModuleService } from '../models/module.model'
import { Site } from '../models/site.model'
import { ModuleType } from '../models/moduleType.model'
import { ModuleConfigFile } from '../models/moduleConfigFile.model'
import { HybridOSVersion } from '../models/hybridOSVersion.model'
import { Customer } from '../models/customer.model'
import { ControllerVersion } from '../models/controllerVersion.model'
import { ControllerType } from '../models/controllerType.model'
import { Controller } from '../models/controller.model'
import { ConfigVersion } from '../models/configVersion.model'
import { ConfigLocationType } from '../models/configLocationType.model'
import { ConfigLocation } from '../models/configLocation.model'

import { getModelToken as getMongooseModelToken } from '@nestjs/mongoose'
import { Model } from 'mongoose'
import { ConfigFile, ConfigFileDocument } from '../schemas/configFile.schema'
import { UsersService } from '../../users/users.service'
import { LifecycleStatus } from '../models/lifecycle.model'
import { ConfigHelperService } from '../configHelper.service'
import { ModuleControllerVersion } from '../models/moduleControllerVersion.model'

describe('ConfigPullerService', () => {
    let service: ConfigPullerService

    let helperService: ConfigHelperService
    let configLocationRepo: typeof ConfigLocation
    let mongoConfigFile: Model<ConfigFileDocument>

    const numSampleFiles = 4

    const goodFilePaths = [
        './src/configFiles/exampleConfigSets/exampleConfigFiles/ess_controller/config/metrics/metrics.json',
        './src/configFiles/exampleConfigSets/exampleConfigFiles/ess_controller/config/storage/storage.json',
        './src/configFiles/exampleConfigSets/exampleConfigFiles/site_controller/config/scheduler/configuration.json',
        './src/configFiles/exampleConfigSets/exampleConfigFiles/site_controller/config/scheduler/modes.json',
    ]
    const badFilePaths = [
        './src/configFiles/exampleConfigSets/exampleConfigFiles/ess_controller/config/metrics/metrics.json',
        './src/configFiles/exampleConfigSets/exampleConfigFiles/ess_controller/config/storage/storage.json',
        'badFilePath',
        './src/configFiles/exampleConfigSets/exampleConfigFiles/site_controller/config/scheduler/modes.json',
    ]
    const directory: string = './src/configFiles/exampleConfigSets/exampleConfigFiles/'

    beforeEach(async () => {
        const db = new Sequelize({
            dialect: 'sqlite',
            storage: ':memory:',
            logging: false,
        })
        await db.sync()

        const module = await Test.createTestingModule({
            providers: [
                ConfigPullerService,
                {
                    provide: ConfigHelperService,
                    useValue: {
                        getJSONFiles: jest.fn().mockReturnValue(goodFilePaths),
                        processFile: jest.fn(),
                        findOrCreate: jest
                            .fn()
                            .mockImplementation((repo, attributes, transaction) => {
                                return { ...getAttributes, id: 1 }
                            }),
                        processFileContents: jest.fn().mockImplementation((contents) => contents),
                    },
                },
                {
                    provide: getModelToken(ConfigLocation),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                        findAll: jest.fn(),
                    },
                },
                {
                    provide: getModelToken(ConfigLocationType),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ConfigVersion),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(Controller),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ControllerType),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ControllerVersion),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(Customer),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(HybridOSVersion),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ModuleConfigFile),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ModuleType),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(Site),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: UsersService,
                    useValue: {
                        findOrCreate: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(LifecycleStatus),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ModuleService),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getModelToken(ModuleControllerVersion),
                    useValue: {
                        findOne: jest.fn(),
                        create: jest.fn().mockImplementation(async () => {
                            return { id: 1 }
                        }),
                    },
                },
                {
                    provide: getMongooseModelToken(ConfigFile.name),
                    useValue: {
                        deleteMany: jest.fn(),
                        create: jest.fn().mockResolvedValue({ _id: '1234' }),
                        findById: jest.fn().mockResolvedValue({
                            name: 'fileName',
                            // eslint-disable-next-line quotes
                            fileContents: '{ "field1": true, "field2": 3 }',
                        }),
                    },
                },
                {
                    provide: Sequelize,
                    useValue: {
                        ...db,
                        transaction: jest.fn().mockResolvedValue({
                            commit: jest.fn(),
                            rollback: jest.fn(),
                        }),
                    },
                },
            ],
        }).compile()

        service = module.get<ConfigPullerService>(ConfigPullerService)
        helperService = module.get<ConfigHelperService>(ConfigHelperService)
        configLocationRepo = module.get<typeof ConfigLocation>(getModelToken(ConfigLocation))
        mongoConfigFile = module.get<Model<ConfigFileDocument>>(
            getMongooseModelToken(ConfigFile.name)
        )
    })

    it('should be defined', () => {
        expect(service).toBeDefined()
    })

    describe('test saveFilesToDatabase', () => {
        it('should save a new version and 4 configFiles to database', async () => {
            // assert that 4 files are saved to the database
            expect(await service.saveFilesToDatabase(goodFilePaths)).toBe(numSampleFiles)
        })
        it('should rollback transaction after the third configFile fails', async () => {
            await expect(service.saveFilesToDatabase(badFilePaths)).rejects.toThrowError()
        })
    })

    describe('test pullConfigsFromFileSystem', () => {
        it('should pull configs successfully', async () => {
            expect(await service.pullConfigsFromFileSystem(directory)).toBe(numSampleFiles)
        })

        it('should return 0 configs saved when no files are found', async () => {
            jest.spyOn(helperService, 'getJSONFiles').mockReturnValueOnce([])

            expect(await service.pullConfigsFromFileSystem(directory)).toBe(0)
        })

        it('should throw an error when json files are invalid', async () => {
            jest.spyOn(helperService, 'processFile').mockRejectedValueOnce(
                new Error('invalid json')
            )

            await expect(service.pullConfigsFromFileSystem(directory)).rejects.toThrowError()
        })
    })

    describe('test pullConfigsFromMongo', () => {
        it('should process all files successfully', async () => {
            jest.spyOn(configLocationRepo, 'findAll').mockResolvedValue([
                {
                    name: 'file1',
                    'modules.id': 12345,
                },
                {
                    name: 'file2',
                    'modules.id': 3456,
                },
            ] as any)

            expect(await service.pullConfigsFromMongo()).toBe(2)
        })

        it('should throw an error when file is not found in mongo', async () => {
            jest.spyOn(configLocationRepo, 'findAll').mockResolvedValue([
                {
                    name: 'file1',
                    'modules.id': 12345,
                },
                {
                    name: 'file2',
                    'modules.id': 3456,
                },
            ] as any)
            jest.spyOn(mongoConfigFile, 'findById').mockImplementation(() => {
                throw new Error('cannot find file')
            })

            await expect(service.pullConfigsFromMongo()).rejects.toThrowError()
        })

        it('should throw an error when fileContents are not valid json', async () => {
            jest.spyOn(helperService, 'processFileContents').mockRejectedValueOnce(
                new Error('invalid json')
            )

            jest.spyOn(configLocationRepo, 'findAll').mockResolvedValue([
                {
                    name: 'file1',
                    'modules.id': 12345,
                },
                {
                    name: 'file2',
                    'modules.id': 3456,
                },
            ] as any)
            jest.spyOn(mongoConfigFile, 'findById').mockResolvedValue({
                name: 'badFile',
                fileContents: {},
            })

            await expect(service.pullConfigsFromMongo()).rejects.toThrowError()
        })
    })
})

import { Injectable } from '@nestjs/common'
import { InjectModel } from '@nestjs/sequelize'
import { Op, Sequelize } from 'sequelize'
import { ConfigFileMetadata } from './interfaces/configFileMetadata.interface'
import { ControllerVersionMetadata } from './interfaces/controllerVersionMetadata.interface'
import { ControllerVersion } from './models/controllerVersion.model'
import { LifecycleStatus } from './models/lifecycle.model'
import { ModuleService } from './models/module.model'
import { ModuleConfigFile } from './models/moduleConfigFile.model'
import { ConfigHistoryResponse } from './dtos/configHistory.response.dto'
import { User } from '../users/user.model'

@Injectable()
export class ConfigHistoryService {
    constructor(
        @InjectModel(ModuleConfigFile) private moduleConfigFileRepository: typeof ModuleConfigFile,
        @InjectModel(ModuleService) private moduleServiceRepository: typeof ModuleService,
        @InjectModel(ControllerVersion) private controllerVersion: typeof ControllerVersion
    ) {}
    async getHistory(): Promise<ConfigHistoryResponse> {
        const activeVersion: ControllerVersionMetadata = await this.getActiveVersion()
        const files: ConfigFileMetadata[] = await this.getFiles()

        return {
            activeVersion,
            files,
        }
    }
    getActiveVersion = async (): Promise<ControllerVersionMetadata> => {
        // this query assumes there is only one controller in the database
        const { fk_controller_id, ...active } = await this.controllerVersion.findOne({
            raw: true,
            attributes: [
                'id',
                'version',
                'dateRetired',
                'dateCreated',
                'dateActive',
                'fk_controller_id',
                [Sequelize.col('fk_lifecycle.name'), 'lifeCycleStatus'],
                [Sequelize.col('fk_uploader.username'), 'creatorName'],
            ],
            include: [
                { model: LifecycleStatus, attributes: [] },
                { model: User, attributes: [] },
            ],
        })

        const archivedControllers = await this.controllerVersion.findAll({
            raw: true,
            where: { fk_controller_id: fk_controller_id, id: { [Op.ne]: active.id } },
            attributes: [
                'id',
                'version',
                'dateRetired',
                'dateCreated',
                'dateActive',
                [Sequelize.col('fk_lifecycle.name'), 'lifeCycleStatus'],
                [Sequelize.col('fk_uploader.username'), 'creatorName'],
            ],
            include: [
                { model: LifecycleStatus, attributes: [] },
                { model: User, attributes: [] },
            ],
            paranoid: false,
        })
        archivedControllers.sort((a, b) => b.version - a.version)

        const activeVersion: ControllerVersionMetadata = {
            ...active,
            archiveVersions: archivedControllers,
        }
        return activeVersion
    }
    getFiles = async (): Promise<ConfigFileMetadata[]> => {
        const configs: ConfigFileMetadata[] = await this.moduleConfigFileRepository
            .findAll({
                raw: true,
                attributes: [
                    'id',
                    'name',
                    'version',
                    'dateRetired',
                    'dateCreated',
                    'dateActive',
                    [Sequelize.col('fk_lifecycle.name'), 'lifeCycleStatus'],
                    [Sequelize.col('fk_uploader.username'), 'creatorName'],
                    [Sequelize.col('fk_module.name'), 'moduleName'],
                ],
                include: [
                    { model: LifecycleStatus, attributes: [] },
                    { model: User, attributes: [] },
                    { model: ModuleService, attributes: [] },
                ],
            })
            .then(async (current: ConfigFileMetadata[]) => {
                const allConfigs: ConfigFileMetadata[] = await Promise.all(
                    current.map(async (config: ConfigFileMetadata) => {
                        config.archiveVersions = await this.getArchiveFiles(config.name, config.id)

                        return config
                    })
                )
                allConfigs.sort((a, b) => (a.dateCreated > b.dateCreated ? -1 : 1))

                return allConfigs
            })

        return configs
    }
    getArchiveFiles = async (name: string, activeId: number): Promise<ConfigFileMetadata[]> => {
        const archivedConfigs: ConfigFileMetadata[] = await this.moduleConfigFileRepository
            .findAll({
                raw: true,
                where: {
                    name: name,
                    id: { [Op.ne]: activeId },
                },
                attributes: [
                    'id',
                    'name',
                    'version',
                    'dateRetired',
                    'dateCreated',
                    'dateActive',
                    [Sequelize.col('fk_lifecycle.name'), 'lifeCycleStatus'],
                    [Sequelize.col('fk_uploader.username'), 'creatorName'],
                    [Sequelize.col('fk_module.name'), 'moduleName'],
                ],
                include: [
                    { model: LifecycleStatus, attributes: [] },
                    { model: User, attributes: [] },
                    { model: ModuleService, attributes: [] },
                ],
                paranoid: false,
            })
            .then((configs) => configs.sort((a, b) => b.version - a.version))

        return archivedConfigs
    }
}

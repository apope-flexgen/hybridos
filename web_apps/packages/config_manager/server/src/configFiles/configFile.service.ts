/* eslint-disable max-statements */
import { Injectable } from '@nestjs/common'
import { InjectModel } from '@nestjs/sequelize'
import { Transaction } from 'sequelize'
import { Sequelize } from 'sequelize-typescript'
import { User } from '../users/user.model'
import { ConfigHelperService } from './configHelper.service'
import { ConfigPullerService } from './configPuller.service'
import { Controller } from './models/controller.model'
import { ControllerVersion } from './models/controllerVersion.model'
import { ModuleService } from './models/module.model'
import { ModuleConfigFile } from './models/moduleConfigFile.model'
import { ModuleControllerVersion } from './models/moduleControllerVersion.model'

@Injectable()
export class ConfigFileService {
    constructor(
        @InjectModel(ModuleConfigFile) private moduleConfigFileRepository: typeof ModuleConfigFile,
        @InjectModel(ControllerVersion)
        private controllerVersionRepository: typeof ControllerVersion,
        @InjectModel(Controller) private controllerRepository: typeof Controller,
        @InjectModel(ModuleControllerVersion)
        private moduleControllerVersionRepository: typeof ModuleControllerVersion,

        private readonly configHelperService: ConfigHelperService,

        private sequelize: Sequelize
    ) {}
    async updateConfig(id: number, fileContents: string) {
        const t = await this.sequelize.transaction()
        try {
            await this.createNewControllerVersion(id, t)

            const newConfig = await this.createNewConfigFile(id, fileContents, t)

            await t.commit()
            return {
                updated: newConfig,
            }
        } catch (error) {
            await t.rollback()
            throw new Error(error)
        }
    }
    createNewConfigFile = async (
        existingID: number,
        fileContents: string,
        transaction: Transaction
    ) => {
        // clone and delete given configFile

        const existing = await this.moduleConfigFileRepository
            .findByPk(existingID, {
                transaction: transaction,
            })
            .then(async (file) => {
                await file.destroy({ transaction: transaction })

                return file
            })

        const newConfig = await this.moduleConfigFileRepository.create(
            {
                name: existing.name,
                version: existing.version + 1,
                file: await this.configHelperService.processFileContents(fileContents),

                fk_module_id: existing.fk_module_id,
                fk_uploader_id: existing.fk_uploader_id,
                fk_lifecycle_id: existing.fk_lifecycle_id,
            },
            { transaction: transaction }
        )

        return newConfig
    }
    createNewControllerVersion = async (configFileID: number, transaction: Transaction) => {
        // create new controller version & config version ?? what is the difference ??
        // TODO: don't assume that there is only one controller present
        const existingControllerVersion = await this.controllerVersionRepository
            .findOne({
                // include: {
                //     model: ModuleControllerVersion,
                //     include: [
                //         // { model: ModuleConfigFile, where: { id: id } },
                //         // { all: true, nested: true },
                //     ],
                // },
                transaction: transaction,
            })
            .then(async (controllerVersion) => {
                await controllerVersion.destroy({ transaction: transaction })

                return controllerVersion
            })

        const newControllerVersion = await this.controllerVersionRepository
            .create(
                {
                    fk_controller_id: existingControllerVersion.fk_controller_id,
                    fk_configVersion_id: existingControllerVersion.fk_configVersion_id,
                    fk_uploader_id: existingControllerVersion.fk_uploader_id,
                    fk_lifecycle_id: existingControllerVersion.fk_lifecycle_id,
                    version: existingControllerVersion.version + 1,
                },
                { transaction: transaction }
            )
            .then(async (controllerVersion) => {
                // populate the new controllerVersion's modules with the previous controllerVersion's modules
                await this.populateControllerVersionModules(
                    existingControllerVersion.id,
                    controllerVersion.id,
                    transaction
                )
            })
    }
    populateControllerVersionModules = async (
        previousID: number,
        futureID: number,
        transaction: Transaction
    ) => {
        const many = await this.moduleControllerVersionRepository.findAll({
            where: { controller_version_id: previousID },
            transaction: transaction,
        })
        for (const manyTable of many) {
            await this.moduleControllerVersionRepository.create(
                {
                    module_id: manyTable.module_id,
                    controller_version_id: futureID,
                },
                { transaction: transaction }
            )
        }
    }
    async getAll() {
        return await this.moduleConfigFileRepository.findAll({ paranoid: false })
    }
}

import { BelongsTo, Column, ForeignKey, HasMany, Model, Table } from 'sequelize-typescript'
import { ConfigLocation } from './configLocation.model'
import { ControllerVersion } from './controllerVersion.model'
import { ModuleConfigFile } from './moduleConfigFile.model'
import { ModuleControllerVersion } from './moduleControllerVersion.model'
import { ModuleType } from './moduleType.model'

@Table
export class ModuleService extends Model {
    @Column
    name: string
    @ForeignKey(() => ModuleType)
    @Column
    fk_moduletype_id: number
    @BelongsTo(() => ModuleType)
    fk_moduletype: ModuleType
    @HasMany(() => ConfigLocation)
    configLocations: ConfigLocation[]
    @HasMany(() => ModuleControllerVersion)
    moduleControllerVersions: ModuleControllerVersion[]
    @HasMany(() => ModuleConfigFile)
    moduleConfigFiles: ModuleConfigFile[]
}

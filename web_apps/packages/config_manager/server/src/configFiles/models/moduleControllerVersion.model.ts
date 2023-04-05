import { BelongsTo, Column, ForeignKey, Model, PrimaryKey, Table } from 'sequelize-typescript'
import { ControllerVersion } from './controllerVersion.model'
import { ModuleService } from './module.model'

@Table
export class ModuleControllerVersion extends Model {
    @BelongsTo(() => ModuleService)
    module: ModuleService
    @ForeignKey(() => ModuleService)
    @PrimaryKey
    @Column
    module_id: number
    @BelongsTo(() => ControllerVersion)
    controller_version: ControllerVersion
    @ForeignKey(() => ControllerVersion)
    @PrimaryKey
    @Column
    controller_version_id: number
}

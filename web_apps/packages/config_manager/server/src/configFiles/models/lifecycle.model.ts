import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { ControllerVersion } from './controllerVersion.model'
import { ModuleConfigFile } from './moduleConfigFile.model'

export type LifecycleStatusEnum =
    | 'uploaded'
    | 'testing'
    | 'historical'
    | 'pending historical'
    | 'rejected'
    | 'active'

@Table
export class LifecycleStatus extends Model {
    @Column
    name: LifecycleStatusEnum
@HasMany(() => ModuleConfigFile)
    configFiles: ModuleConfigFile[]
@HasMany(() => ControllerVersion)
    controllerVersions: ControllerVersion[]
}

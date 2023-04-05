import { BelongsTo, Column, ForeignKey, HasMany, Model, Table } from 'sequelize-typescript'
import { User } from '../../users/user.model'
import { ConfigVersion } from './configVersion.model'
import { Controller } from './controller.model'
import { LifecycleStatus } from './lifecycle.model'
import { ModuleControllerVersion } from './moduleControllerVersion.model'

@Table({
    paranoid: true,
    deletedAt: 'dateRetired',
    createdAt: 'dateCreated',
})
export class ControllerVersion extends Model {
    @Column
    version: number
    @Column
    dateActive: string
    @ForeignKey(() => Controller)
    @Column
    fk_controller_id: number
    @BelongsTo(() => Controller)
    fk_controller: Controller
    @ForeignKey(() => ConfigVersion)
    @Column
    fk_configVersion_id: number
    @BelongsTo(() => ConfigVersion)
    fk_configVersion: ConfigVersion
    @HasMany(() => ModuleControllerVersion)
    moduleControllerVersions: ModuleControllerVersion[]
    @ForeignKey(() => User)
    @Column
    fk_uploader_id: number
    @BelongsTo(() => User)
    fk_uploader: User
    @ForeignKey(() => LifecycleStatus)
    @Column
    fk_lifecycle_id: number
    @BelongsTo(() => LifecycleStatus)
    fk_lifecycle: LifecycleStatus
}

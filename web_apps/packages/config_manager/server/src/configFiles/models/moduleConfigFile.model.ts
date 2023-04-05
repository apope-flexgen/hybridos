import { TEXT } from 'sequelize'
import { BelongsTo, Column, ForeignKey, Model, Table } from 'sequelize-typescript'
import { User } from '../../users/user.model'
import { LifecycleStatus } from './lifecycle.model'
import { ModuleService } from './module.model'

@Table({
    paranoid: true,
    deletedAt: 'dateRetired',
    createdAt: 'dateCreated',
})
export class ModuleConfigFile extends Model {
    @Column
    name: string
@Column
    version: number
@Column(TEXT('long'))
    file: string
@Column
    dateActive: string
@ForeignKey(() => ModuleService)
    @Column
    fk_module_id: number
@BelongsTo(() => ModuleService)
    fk_module: ModuleService
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

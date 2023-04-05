import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { ModuleService } from './module.model'

@Table
export class ModuleType extends Model {
    @Column
    name: string
    @HasMany(() => ModuleService)
    modules: ModuleService[]
}

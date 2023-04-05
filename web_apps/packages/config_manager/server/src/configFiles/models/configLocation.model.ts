import { BelongsTo, Column, ForeignKey, HasMany, Model, Table } from 'sequelize-typescript'
import { ConfigLocationType } from './configLocationType.model'
import { ModuleService } from './module.model'

@Table
export class ConfigLocation extends Model {
    @Column
    name: string
    @ForeignKey(() => ModuleService)
    @Column
    fk_module_id: number
    @BelongsTo(() => ModuleService)
    fk_module: ModuleService
    @ForeignKey(() => ConfigLocationType)
    @Column
    fk_configLocationType_id: number
    @BelongsTo(() => ConfigLocationType)
    fk_configLocationType: ConfigLocationType
}

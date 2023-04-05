import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { Controller } from './controller.model'

@Table
export class HybridOSVersion extends Model {
    @Column
    name: string
    @HasMany(() => Controller)
    controllers: Controller[]
}

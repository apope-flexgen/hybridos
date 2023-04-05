import { BelongsTo, Column, ForeignKey, HasMany, Model, Table } from 'sequelize-typescript'
import { Controller } from './controller.model'
import { Customer } from './customer.model'

@Table
export class Site extends Model {
    @Column
    name: string
    @ForeignKey(() => Customer)
    @Column
    fk_customer_id: number
    @BelongsTo(() => Customer)
    fk_customer: Customer
    @HasMany(() => Controller)
    controllers: Controller[]
}

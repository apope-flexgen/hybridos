import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { Site } from './site.model'

@Table
export class Customer extends Model {
    @Column
    name: string
    @HasMany(() => Site)
    sites: Site[]
}

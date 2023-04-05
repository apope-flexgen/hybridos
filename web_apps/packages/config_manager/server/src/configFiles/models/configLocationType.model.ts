import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { ConfigLocation } from './configLocation.model'

@Table
export class ConfigLocationType extends Model {
    @Column
    name: string
    @HasMany(() => ConfigLocation)
    configLocations: ConfigLocation[]
}

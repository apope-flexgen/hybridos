import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { ControllerVersion } from './controllerVersion.model'

@Table
export class ConfigVersion extends Model {
    @Column
    name: string
    @HasMany(() => ControllerVersion)
    controllerVersions: ControllerVersion[]
}

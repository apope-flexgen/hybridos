import { Column, HasMany, Model, Table } from 'sequelize-typescript'
import { ControllerVersion } from '../configFiles/models/controllerVersion.model'
import { ModuleConfigFile } from '../configFiles/models/moduleConfigFile.model'

@Table
export class User extends Model {
    @Column
    username: string
    @Column
    firstName: string
    @Column
    lastName: string
    @Column({ defaultValue: true })
    isActive: boolean
    @HasMany(() => ModuleConfigFile)
    configFiles: ModuleConfigFile[]
    @HasMany(() => ControllerVersion)
    controllerVersions: ControllerVersion[]
}

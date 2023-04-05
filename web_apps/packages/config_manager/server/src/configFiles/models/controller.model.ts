import { BelongsTo, Column, ForeignKey, HasMany, Model, Table } from 'sequelize-typescript'
import { ControllerType } from './controllerType.model'
import { ControllerVersion } from './controllerVersion.model'
import { HybridOSVersion } from './hybridOSVersion.model'
import { Site } from './site.model'

@Table
export class Controller extends Model {
    @Column
    name: string
    @ForeignKey(() => Site)
    @Column
    fk_site_id: number
    @BelongsTo(() => Site)
    fk_site: Site
    @ForeignKey(() => ControllerType)
    @Column
    fk_controllerType_id: number
    @BelongsTo(() => ControllerType)
    fk_controllerType: ControllerType
    @ForeignKey(() => HybridOSVersion)
    @Column
    fk_hybridOSVersion_id: number
    @BelongsTo(() => HybridOSVersion)
    fk_hybridOSVersion: HybridOSVersion
    @HasMany(() => ControllerVersion)
    controllerVersions: ControllerVersion[]
}

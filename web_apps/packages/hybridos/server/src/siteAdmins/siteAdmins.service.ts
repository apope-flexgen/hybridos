import { Injectable } from '@nestjs/common'
import { InjectModel } from '@nestjs/mongoose'
import { Model } from 'mongoose'
import { AuthRadius } from '../radius/authRadius'
import { SiteAdminsDto } from './dto/create-siteAdmins.dto'
import { RadiusTestDto } from './dto/radiusTest-siteAdmins.dto'
import { SiteAdmin } from './interfaces/siteAdmin.interface'
import { SiteAdminDocument } from './schemas/siteAdmins.schema'

@Injectable()
export class SiteAdminsService {
    constructor(
        @InjectModel('siteadmins')
        private readonly siteAdminsModel: Model<SiteAdminDocument>
    ) {}
    async create(createSiteAdminsDto: SiteAdminsDto): Promise<SiteAdmin> {
        // clear the database
        await this.siteAdminsModel.deleteMany({})

        const createdSiteAdmins = await this.siteAdminsModel.create(
            createSiteAdminsDto
        )
        return createdSiteAdmins
    }
    async find(): Promise<SiteAdmin> {
        const site = await this.siteAdminsModel.findOne({}).exec()

        // if no siteAdmin is found,
        //    create -> save -> return a new siteAdmin with default values
        if (!site) {
            const defaultSiteAdmins = await this.siteAdminsModel.create({})
            return defaultSiteAdmins
        }
        return site
    }
    async radiusTest(radiusSettings: RadiusTestDto): Promise<string> {
        const authRadius = new AuthRadius(
            radiusSettings.ip_address,
            radiusSettings.secret_phrase,
            radiusSettings.port,
            radiusSettings.wait_time
        )

        return new Promise((resolve) => {
            authRadius
                .authenticate(radiusSettings.username, radiusSettings.password)
                .onAccept((decodedPacket) => {
                    const role = authRadius.getAttributeRole(decodedPacket)
                    resolve(role)
                })
                .onReject(() => {
                    resolve(null)
                })
                .onError(() => {
                    resolve(null)
                })
                .onTimeout(() => {
                    resolve(null)
                })
        })
    }
}

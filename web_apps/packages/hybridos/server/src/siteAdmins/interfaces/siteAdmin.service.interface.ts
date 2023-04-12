import { SiteAdminsDto } from '../dto/create-siteAdmins.dto'
import { RadiusTestDto } from '../dto/radiusTest-siteAdmins.dto'
import { SiteAdmin } from './siteAdmin.interface'

export const SITE_ADMINS_SERVICE = 'SiteAdminsService'

export interface ISiteAdminsService {
    create(createSiteAdminsDto: SiteAdminsDto): Promise<SiteAdmin>
    find(): Promise<SiteAdmin>
    radiusTest(radiusSettings: RadiusTestDto): Promise<string>
}

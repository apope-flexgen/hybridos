import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose'
import { Document } from 'mongoose'

export type SiteAdminDocument = SiteAdmin & Document

@Schema()
class PasswordSetting {
    @Prop({ default: false })
    multi_factor_authentication: boolean

    @Prop({ default: false })
    password_expiration: boolean

    @Prop({ default: '8d' })
    password_expiration_interval: string

    @Prop({ default: 8 })
    minimum_password_length: number

    @Prop({ default: 128 })
    maximum_password_length: number

    @Prop({
        default: JSON.stringify(
            /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
        ),
    })
    password_regular_expression: string

    @Prop({ default: 0 })
    old_passwords: number

    @Prop({ default: true })
    lowercase: boolean

    @Prop({ default: true })
    uppercase: boolean

    @Prop({ default: true })
    digit: boolean

    @Prop({ default: true })
    special: boolean
}
const passwordSettingSchema = SchemaFactory.createForClass(PasswordSetting)

@Schema()
class RadiusSetting {
    @Prop({ default: false })
    is_enabled: boolean

    @Prop({ default: '127.0.0.1' })
    ip_address: string

    @Prop({ default: '1812' })
    port: string

    @Prop({ default: 'testing123' })
    secret_phrase: string

    @Prop({ default: 5000 })
    wait_time: number

    @Prop({ default: false })
    is_local_auth_disabled: boolean
}
const radiusSettingSchema = SchemaFactory.createForClass(RadiusSetting)

@Schema()
export class SiteAdmin {
    @Prop({ type: passwordSettingSchema, default: new PasswordSetting() })
    password: PasswordSetting

    @Prop({ type: radiusSettingSchema, default: new RadiusSetting() })
    radius: RadiusSetting
}

export const SiteAdminsSchema = SchemaFactory.createForClass(SiteAdmin)

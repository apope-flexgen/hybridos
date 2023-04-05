import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose'
import { Document } from 'mongoose'

export type ConfigFileDocument = ConfigFile & Document

@Schema()
export class ConfigFile {
    @Prop()
    fileContents: string
    @Prop()
    name: string
}

export const ConfigFileSchema = SchemaFactory.createForClass(ConfigFile)

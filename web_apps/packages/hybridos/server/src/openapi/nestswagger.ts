import { INestApplication } from '@nestjs/common'
import {
    DocumentBuilder,
    OpenAPIObject,
    SwaggerDocumentOptions,
    SwaggerModule,
} from '@nestjs/swagger'
import { writeFileSync } from 'fs'
import { NestSwaggerDescriptions } from './nestswagger.constants'

export class NestSwagger {
    private document: OpenAPIObject
    constructor(private app: INestApplication) {}
    public generateDocumentation(): void {
        const DOC_PATH = './openapi/swagger-spec.json'
        const config = new DocumentBuilder()
            .setTitle(NestSwaggerDescriptions.title)
            .setDescription(NestSwaggerDescriptions.description)
            .setVersion(NestSwaggerDescriptions.version)
            .addTag(NestSwaggerDescriptions.tag)
            .addTag('assets', 'Allows users to get and edit configured assets from dbi')
            .addTag('ercot-override', 'Allows users to get and override ERCOT standard variables')
            .addTag('events', 'Allows users to get a list of filtered events')
            .addTag('scheduler', 'Allows users to get, add, delete, and edit Scheduler events and modes, as well as get and edit the Scheduler configuration')
            .addTag('site', 'Allows users to get, add, delete, and edit Scheduler events and modes, as well as get and edit the Scheduler configuration')
            .addTag('dashboards', 'Allows users to get and edit configured dashboard cards from dbi')
            .addTag('fims', 'Allows users to get a fims one-time-auth token, as well as send fims get, post, set, and delete messages')
            .addTag('layouts', 'Allows users to get and edit configured layouts from dbi')
            .addTag('site-admins', 'Allows users to get and edit site settings (password settings, radius settings), send radius tests')
            .addTag('users', 'Allows users to get, add, edit, read, and delete users')
            .addTag('web-ui-config', 'Allows users to get the full site configuration data object as well as a subset of config data used on the login page')
            .addBasicAuth({type: 'http'}, 'basicAuth')
            .addBearerAuth({type: 'http', bearerFormat: 'JWT'}, 'bearerAuth')
            .build()

        const options: SwaggerDocumentOptions = {
            operationIdFactory: (controllerKey: string, methodKey: string) =>
                methodKey,
        }
        this.document = SwaggerModule.createDocument(this.app, config, options)

        this.writeToFile(DOC_PATH)
    }
    private writeToFile(path: string): void {
        try {
            writeFileSync(path, JSON.stringify(this.document, null, '\t'))
        } catch (error) {
            console.log(
                `Error saving OpenAPI documentation to filesystem: ${error}`
            )
        }
    }
}

import { INestApplication } from '@nestjs/common'
import {
    DocumentBuilder,
    OpenAPIObject,
    SwaggerDocumentOptions,
    SwaggerModule,
} from '@nestjs/swagger'
import { writeFileSync } from 'fs'

export class NestSwagger {
    private document: OpenAPIObject
    constructor(
        private app: INestApplication // private env: AppEnv TODO: Add this env.
    ) {}
    public generateDocumentation(): void {
        const DOC_PATH = './openapi/swagger-spec.json' // TODO: Get this from AppEnv config.
        const config = new DocumentBuilder()
            .setTitle('HybridOS Web Server') // TODO: Get this from AppEnv config.
            .setDescription('Web Server REST API Documentation.') // TODO: Get this from AppEnv config.
            .setVersion('1.0') // TODO: Get this from AppEnv config.
            .addTag('nest_web_server') // TODO: Get this from AppEnv config.
            .build()

        const options: SwaggerDocumentOptions = {
            operationIdFactory: (controllerKey: string, methodKey: string) => methodKey,
        }
        this.document = SwaggerModule.createDocument(this.app, config, options)

        this.writeToFile(DOC_PATH)
    }
    private writeToFile(path: string): void {
        try {
            writeFileSync(path, JSON.stringify(this.document, null, '\t'))
        } catch (error) {
            console.log(`Error saving OpenAPI documentation to filesystem: ${error}`)
        }
    }
}

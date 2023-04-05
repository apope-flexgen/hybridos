import { Injectable } from '@nestjs/common'
import * as fs from 'fs'
import * as glob from 'glob'
import { Transaction } from 'sequelize'

@Injectable()
export class ConfigHelperService {
    getJSONFiles = (directory: string): string[] => {
        const filePaths = glob.sync(directory + '/**/*.json', { absolute: true })

        return filePaths
    }
    processFile = async (filePath: string): Promise<Buffer> => {
        const fileContents = fs.readFileSync(filePath, { encoding: 'utf-8' })

        try {
            return await this.processFileContents(fileContents)
        } catch (error) {
            throw new Error(`invalid JSON in file: ${filePath}`)
        }
    }
    processFileContents = async (fileContents: string): Promise<Buffer> => {
        const validJSON = this.isValidJSONObject(fileContents)
        const sortedJSON = this.sortJSONProperties(validJSON)
        return Buffer.from(JSON.stringify(sortedJSON))
    }
    isValidJSONObject = (jsonString: string): object => {
        const jsonObject: object = JSON.parse(jsonString)
        if (jsonObject && jsonObject.constructor.name === 'Object') {
            return jsonObject
        }
        throw new Error('invalid JSON')
    }
    sortJSONProperties = (jsonObject: object): object => {
        const sortedJSON = Object.keys(jsonObject)
            .sort()
            .reduce((obj, key) => {
                if (jsonObject[key].constructor.name === 'Object') {
                    obj[key] = this.sortJSONProperties(jsonObject[key])
                } else {
                    obj[key] = jsonObject[key]
                }
                return obj
            }, {})
        return sortedJSON
    }
    findOrCreate = async (repo: any, options: any, t: Transaction): Promise<typeof repo> => {
        const existing = await repo.findOne({ where: options, transaction: t })
        if (existing) return existing

        const created = await repo.create(options, { transaction: t })
        return created
    }
    printRepo = async (repo: any) => {
        await repo.findAll({ include: { all: true } }).then((results: any[]) => {
            results.forEach((result: any) => {
                console.log(result.toJSON())
            })
        })
    }
}

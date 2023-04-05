/* eslint-disable max-nested-callbacks */
import { Test } from '@nestjs/testing'
import { ConfigHelperService } from '../configHelper.service'
import * as fs from 'fs'

describe('ConfigHelperService', () => {
    let service: ConfigHelperService

    const goodDirectory: string = './src/configFiles/exampleConfigSets/exampleConfigFiles/'
    const badDirectory: string = './src/configFiles/exampleConfigSets/badConfigs/'

    beforeEach(async () => {
        const module = await Test.createTestingModule({
            providers: [ConfigHelperService],
        }).compile()

        service = module.get<ConfigHelperService>(ConfigHelperService)
    })

    it('should be defined', () => {
        expect(service).toBeDefined()
    })

    describe('test getJSONFiles', () => {
        it('should get file paths of the 4 files in exampleConfigFiles directory', () => {
            const expected: string[] = [
                'ess_controller/config/metrics/metrics.json',
                'ess_controller/config/storage/storage.json',
                'site_controller/config/scheduler/configuration.json',
                'site_controller/config/scheduler/modes.json',
            ]

            const result: string[] = service.getJSONFiles(goodDirectory)

            expect(result.length).toBe(expected.length)
            result.forEach((path, index) => {
                expect(path).toContain(expected[index])
            })
        })

        it('should return an empty array when no json in directory', () => {
            const directory: string = './src/configFiles/models/'

            const result: string[] = service.getJSONFiles(directory)

            expect(result.length).toBe(0)
        })

        it('should return an empty array when directory is not found', () => {
            const directory: string = './src/fakeDirectory/'

            const result: string[] = service.getJSONFiles(directory)

            expect(result.length).toBe(0)
        })
    })

    describe('test processFile', () => {
        it('should process all files successfully', async () => {
            const validFiles: string[] = service.getJSONFiles(goodDirectory)

            validFiles.forEach(async (file) => {
                expect(await service.processFile(file)).toBeTruthy()
            })
        })

        it('should throw an error on each invalid json file', () => {
            const invalidFiles: string[] = service.getJSONFiles(badDirectory)

            invalidFiles.forEach(async (file) => {
                await expect(service.processFile(file)).rejects.toThrowError()
            })
        })
    })

    describe('test isValidJSONObject', () => {
        it('should process all files successfully', async () => {
            const validFiles: string[] = service.getJSONFiles(goodDirectory)

            validFiles.forEach((file) => {
                const fileContents = fs.readFileSync(file, { encoding: 'utf-8' })
                expect(service.isValidJSONObject(fileContents)).toBeTruthy()
            })
        })

        it('should throw an error on each invalid json file', () => {
            const invalidFiles: string[] = service.getJSONFiles(badDirectory)

            invalidFiles.forEach((file) => {
                const fileContents = fs.readFileSync(file, { encoding: 'utf-8' })
                expect(() => service.isValidJSONObject(fileContents)).toThrowError()
            })
        })
    })

    describe('test sortJSONProperties', () => {
        it('should successfully sort each json object', () => {
            const obj = {
                first: {
                    a: true,
                    c: false,
                    b: {
                        luckyCharms: true,
                        capnCrunch: false,
                        appleJacks: true,
                    },
                },
                alpha: ['d', 'c', 'a', 'b'],
                second: [124, 56, 3, 1234],
                beta: true,
                gamma: {
                    omega: ['abc', 123],
                    tau: true,
                },
                omicron: ['1234', 1233, 'abc'],
            }
            const sortedObj = {
                alpha: ['d', 'c', 'a', 'b'],
                beta: true,
                first: {
                    a: true,
                    b: {
                        appleJacks: true,
                        capnCrunch: false,
                        luckyCharms: true,
                    },
                    c: false,
                },
                gamma: {
                    omega: ['abc', 123],
                    tau: true,
                },
                omicron: ['1234', 1233, 'abc'],
                second: [124, 56, 3, 1234],
            }

            expect(JSON.stringify(service.sortJSONProperties(obj))).not.toStrictEqual(
                JSON.stringify(obj)
            )
            expect(JSON.stringify(service.sortJSONProperties(obj))).toStrictEqual(
                JSON.stringify(sortedObj)
            )
        })
    })
})

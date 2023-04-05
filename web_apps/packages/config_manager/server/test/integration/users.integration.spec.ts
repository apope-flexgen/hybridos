import { INestApplication } from '@nestjs/common'
import { Test, TestingModule } from '@nestjs/testing'
import * as request from 'supertest'

import { AppModule } from '../../src/app.module'

describe('Users (Integration)', () => {
    let app: INestApplication

    const VALID_USER = {
        firstName: 'Tyrion',
        lastName: 'Lannister',
        isActive: false,
    }
    const USER_ID = 1

    beforeAll(async () => {
        const moduleFixture: TestingModule = await Test.createTestingModule({
            imports: [AppModule],
        }).compile()

        app = moduleFixture.createNestApplication()
        await app.init()
    })

    it('Should create a User', async () => {
        const res = await request(app.getHttpServer()).post('/users').send(VALID_USER)

        expect(res.status).toBe(201)
        expect(res.body).toEqual(
            expect.objectContaining({
                id: USER_ID,
                firstName: VALID_USER.firstName,
                lastName: VALID_USER.lastName,
                isActive: VALID_USER.isActive,
            })
        )
    })

    it('Should read a User', async () => {
        const URL = `/users/${USER_ID}`

        const res = await request(app.getHttpServer()).get(URL)

        expect(res.status).toBe(200)
        expect(res.body).toEqual(
            expect.objectContaining({
                id: USER_ID,
                firstName: VALID_USER.firstName,
                lastName: VALID_USER.lastName,
                isActive: VALID_USER.isActive,
            })
        )
    })

    it('Should read all users', async () => {
        const res = await request(app.getHttpServer()).get('/users')

        expect(res.status).toBe(200)
        expect(res.body).toHaveLength(1)
        expect(res.body[0]).toEqual(
            expect.objectContaining({
                id: USER_ID,
                firstName: VALID_USER.firstName,
                lastName: VALID_USER.lastName,
                isActive: VALID_USER.isActive,
            })
        )
    })

    afterAll(async () => {
        await app.close()
    })
})

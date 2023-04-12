import { Test } from '@nestjs/testing'

import { SiteAdminsController } from './siteAdmins.controller'
import { SiteAdminsDto } from './dto/create-siteAdmins.dto'
import { SITE_ADMINS_SERVICE } from './interfaces/siteAdmin.service.interface'
import { User } from 'src/users/dtos/user.dto'

describe('SiteAdminsController', () => {
    let controller: SiteAdminsController
    const defaultSiteAdmins: SiteAdminsDto = {
        password: {
            multi_factor_authentication: false,
            password_expiration: false,
            minimum_password_length: 8,
            maximum_password_length: 128,
            password_expiration_interval: '8d',
            old_passwords: 0,
            password_regular_expression: JSON.stringify(
                /^(?=.*[a-z])(?=.*[A-Z])(?=.*[0-9])(?=.*[!"#\$%&'\*\+,\.\/:;=\\?@\^`\|~])/
            ),

            lowercase: true,
            uppercase: true,
            digit: true,
            special: true,
        },
        radius: {
            is_enabled: false,
            ip_address: '127.0.0.1',
            port: '1812',
            secret_phrase: 'testing123',
            wait_time: 5000,
            is_local_auth_disabled: false,
        },
    }

    beforeEach(async () => {
        const module = await Test.createTestingModule({
            controllers: [SiteAdminsController],
            providers: [
                {
                    provide: SITE_ADMINS_SERVICE,
                    useValue: {
                        find: jest.fn().mockResolvedValue(defaultSiteAdmins),
                        create: jest.fn().mockImplementation((siteAdmin: SiteAdminsDto) => {
                            return Promise.resolve({ ...siteAdmin })
                        }),
                    },
                },
            ],
        }).compile()

        controller = module.get<SiteAdminsController>(SiteAdminsController)
    })

    it('should be defined', () => {
        expect(controller).toBeDefined()
    })

    describe('find', () => {
        it('should return the current siteAdmins', async () => {
            expect(controller.find()).resolves.toEqual(defaultSiteAdmins)
        })
    })

    describe('create', () => {
        it('should return the inserted siteAdmins', async () => {
            expect(await controller.create(defaultSiteAdmins, new User())).toEqual({
                ...defaultSiteAdmins,
            })
        })
    })
})

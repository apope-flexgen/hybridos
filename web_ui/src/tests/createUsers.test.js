import dotenv from "dotenv";
dotenv.config()

const username = process.env.JEST_USERNAME;
const password = process.env.JEST_PASSWORD;

import puppeteer from "puppeteer";

const SITE = "http://localhost";

// const SITE = "https://172.16.1.80";

describe("HybridOS Web_UI e2e tests", () => {
    let browser;
    let page;

    let alertPopped;
    let alertMessage;
    const resetAlert = () => {
        alertPopped = false;
        alertMessage = "";
    };

    beforeAll(async () => {
        try {
            browser = await puppeteer.launch({
                args:[
                    '--start-maximized' // you can also use '--start-fullscreen'
                 ],
                headless: false,
                ignoreHTTPSErrors: true,
                defaultViewport: null
            });
        } catch (e) {
            console.info("Unable to launch browser mode in sandbox mode. Lauching Chrome without sandbox.");
            browser = await puppeteer.launch({
                ignoreHTTPSErrors: true,
                args: ['--no-sandbox']
            });
        }
        
        page = await browser.newPage();
        page.on("dialog", async (dialog) => {
            alertMessage = dialog.message();
            alertPopped = true;
            await dialog.dismiss();
        });
        await page.goto(SITE);
    });
    
    describe("login page", () => {
        
        // not secure
        describe("with initial credentials", () => {
            jest.setTimeout(20000);
            it("should load dashboard", async () => {
                await page.waitForSelector("#username");
                await page.click("#username");
                await page.type("#username", username);
                await page.click("#password");
                await page.type("#password", password);
                await page.click("#login-button");
                await page.waitForSelector("#page-title");
                const text = await page.$eval(
                    "#page-title",
                    (e) => e.textContent
                );
                expect(text).toBe("Dashboard");
            });
        });

        describe("go to admin page", () => {
            jest.setTimeout(20000);
            it("should load user admin", async () => {
                await page.waitForSelector('[href="/administration/useradministration"]')
                await page.click('[href="/administration/useradministration"]')
                const newText = await page.$eval(
                    "#page-title",
                    (e) => e.textContent
                );
                expect(newText).toBe("Administration / User Administration");
                await page.waitForSelector("#user-roles");
                await page.click("#user-roles");
                await page.waitForSelector("#developer");
                await page.click("#developer");
                await page.waitForTimeout(1000)

            });
        });
        const userTypes = ['developer', 'admin', 'user', 'observer', 'rest']
        userTypes.map(type => {
            describe("create " + type, () => {
                jest.setTimeout(40000);
                it("should create " + type, async () => {
                    await page.click("#add-user");
                    await page.waitForSelector(".roles");
                    let roles = await page.evaluate(() => {
                        const elements = document.getElementsByClassName('roles');
                        let result = [];
                        for (let i = 0; i < elements.length; i++) {
                            let element = elements[i].textContent
                            result.push((element.split(' ').join('')).toLowerCase());
                            }
                        return result;
                    });
                    await page.waitForSelector('#'+roles[roles.length-1])
                    await page.click('#'+roles[roles.length-1]);
                    await page.waitForSelector('[value="New User"]');
                    await page.click('[value="New User"]', {clickCount: 3});
                    await page.keyboard.press('Backspace');
                    await page.type('[value=""]', type + 'username')
                    await page.click('[type="password"]');                 
                    await page.type('[type="password"]','password1A!'); 
                    if (type !== 'developer') {
                        await page.click('#new-role');

                    await page.click("[data-value=" + type + "]");
                } 
                    await page.click('#save-button');
                    await page.waitForSelector('[identifier="check-button"]');
                    let checks = await page.$$('[identifier="check-button"]')
                    await page.waitForTimeout(1000)
                    await checks[0].click()
                    await page.waitForTimeout(1000)
                    await page.click("#save-button");
                    await page.waitForTimeout(1000)
                });
                });
                })
                userTypes.map(type => {
                    describe("test logins " + type, () => {
                        jest.setTimeout(40000);
                        it("should test logins " + type, async () => {
                            await page.waitForTimeout(1000)
                            await page.waitForSelector("#logout-button");
                            await page.click("#logout-button");
                            await page.waitForSelector("#username");
                            await page.click("#username");
                            await page.type("#username", type + 'username')
                            await page.click("#password");
                            await page.type("#password", 'password1A!');
                            await page.click("#login-button");
                            await page.waitForSelector("#page-title");
                            const text = await page.$eval(
                                "#page-title",
                                (e) => e.textContent
                            );
                            expect(text).toBe("Dashboard");
                            });
                        });
                        })
                        describe("with initial credentials", () => {
                            jest.setTimeout(20000);
                            it("should not work", async () => {
                                await page.waitForTimeout(1000)
                                await page.waitForSelector("#logout-button");
                                await page.click("#logout-button");
                                await page.waitForSelector("#username");
                                await page.click("#username");
                                await page.type("#username", username);
                                await page.click("#password");
                                await page.type("#password", password);
                                await page.click("#login-button");
                                await page.reload()
                                await page.waitForTimeout(1000)
                                expect(alertPopped).toBe(true);
                            });
                            it('with message "API: unauthorized"', () => {
                                expect(alertMessage).toBe("API: unauthorized");
                            });
                        });
                afterAll(() => {
                    browser.close();
                });
                });
            });


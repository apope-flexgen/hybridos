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
                ignoreHTTPSErrors: true
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
    });

    describe("login page", () => {
        // not secure
        describe("with initial credentials", () => {
            jest.setTimeout(20000);
            it("should load dashboard", async () => {
                await page.goto(SITE);
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

        describe("with incorrect credentials", () => {
            it("should display an alert", async () => {
                await page.goto(SITE);
                await page.waitForSelector("#login-button");
                await page.click("#login-button");
                expect(alertPopped).toBe(true);
            });
            
            it('with message "API: unauthorized"', () => {
                expect(alertMessage).toBe("API: unauthorized");
            });
        });
    });

    afterAll(() => {
        browser.close();
    });
});

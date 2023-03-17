import dotenv from "dotenv";
dotenv.config()

const username = process.env.DEV_USERNAME;
const password = process.env.DEV_PASSWORD;

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
        describe("with developer credentials", () => {
            jest.setTimeout(80000);
            it("should load dashboard", async () => {
                await page.waitForSelector("#username");
                await page.click("#username");
                await page.type("#username", username);
                await page.click("#password");
                await page.type("#password", password);
                await page.click("#login-button");
            });
        });
    });
    describe("go to UI Configuration page", () => {
        jest.setTimeout(80000);
        it("should load user config page", async () => {
            await page.waitForSelector("#ui-config")
            await page.click("#ui-config")
        });
    });
    describe("try to add layout tabs", () => {
        jest.setTimeout(80000);
        it("should add drawer tabs to righthand nav bar", async () => {
            const button = await page.waitForSelector("#page-selector-id");
            await page.click("#page-selector-id");
            await page.waitForSelector("#layout");
            await page.click("#layout");
            await page.waitForTimeout(1000);
            await page.waitForSelector("#add-item-button");
            await page.click("#add-item-button");
            await page.waitForTimeout(1000);
            await page.waitForSelector("#num-units");
            await page.click("#num-units");
            await page.type("#num-units", '1');
            await page.waitForTimeout(1000);
            await page.waitForSelector('#submit-add-item');
            await page.click('#submit-add-item');
            await page.waitForTimeout(2000);
            let tabs = await page.evaluate(() => {
                const elements = document.getElementsByClassName('ui-list-item');
                let result = [];
                for (let i = 0; i < elements.length; i++) {
                    let element = elements[i].textContent
                    result.push((element.split(' ').join('')).toLowerCase());
                    }
                return result;
            });
            await page.waitForTimeout(1000)
            await page.waitForSelector('#'+tabs[tabs.length-1])
            await page.click('#'+tabs[tabs.length-1]);  
            await page.waitForSelector('[value="New Item"]');
            await page.click('[value="New Item"]', {clickCount: 3});
            await page.keyboard.press('Backspace');
            await page.waitForTimeout(1000)
            await page.type('[value=""]', 'testTab')
            await page.waitForTimeout(1000)
            await page.keyboard.press('Tab')
            await page.type('[value=""','testKey'); 
            await page.waitForTimeout(1000);
            await page.waitForTimeout(2000);
            await page.click('#ui-config-save');
            await page.waitForTimeout(2000);
            await page.waitForSelector('[identifier="check-button"]');
            let checks = await page.$$('[identifier="check-button"]');
            await page.waitForTimeout(1000);
            await checks[0].click();
            await page.waitForTimeout(1000);
            // TODO: shouldn't have to reload to see new drawertab
            await page.reload();
            await page.waitForTimeout(1000);
            const newDrawerTab = await page.$x("//*[contains(text(), 'testTab')]");
            expect(newDrawerTab);
        });
    });
});
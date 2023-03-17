import dotenv from "dotenv";
dotenv.config()

import puppeteer from "puppeteer";

const SITE = "http://localhost";

// const SITE = "https://172.16.1.80";

describe("Run all tests", () => {
    require('./createUsers.test')
    require('./layoutConfig.test')    
});

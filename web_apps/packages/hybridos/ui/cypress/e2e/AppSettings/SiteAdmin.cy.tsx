/* eslint-disable max-statements */
import { mfaMock } from 'src/mocks/pages/Auth/MFA.mock';
import { loginMfaMock, loginMock, loginPassExpMock } from 'src/mocks/pages/Auth/Login.mock';
import { passExpMock } from 'src/mocks/pages/Auth/PassExp.mock';

// TODO: update to use logout button rather than manually clearing cookies when logout button is in
describe('<SiteAdmin />', () => {
  beforeEach(() => {
    cy.visit('/');
    cy.window().then((window) => {
      const { worker } = window.msw;
      worker.use(loginMock);
    });
    cy.get('input').first().type('fgusername');
    cy.get('input').last().type('fgpassword');
    cy.get('button').as('loginButton').click();
    cy.wait(250);
    cy.visit('/siteadmin');
    cy.wait(250);
  });

  it('SUCCESS: Enable MFA', () => {
    cy.get('#enable_mfa').click();
    cy.get('#save_button').click();
    cy.wait(250);

    cy.clearCookies();
    cy.wait(400);
    cy.clearLocalStorage();
    cy.wait(400);
    cy.visit('/');

    cy.window().then((window) => {
      const { worker } = window.msw;
      worker.use(loginMfaMock, mfaMock);
    });

    cy.get('input').first().type('fgusername');
    cy.get('input').last().type('fgpassword');
    cy.get('button').as('loginButton').click();
    cy.wait(250);
    cy.get('input').last().type('1234');
    cy.get('button').last().as('codeButton').click();
    cy.wait(250);
    cy.getCookie('refreshToken').should('have.property', 'value', 'mfa-refresh-token');
  });

  it('SUCCESS: Enable Password Expiration', () => {
    cy.get('#enable_password_expiration').click();
    cy.get('#save_button').click();
    cy.wait(250);

    cy.clearCookies();
    cy.wait(400);
    cy.clearLocalStorage();
    cy.wait(400);
    cy.visit('/');

    cy.window().then((window) => {
      const { worker } = window.msw;
      worker.use(loginPassExpMock, passExpMock);
    });

    cy.get('input').first().type('fgusername');
    cy.get('input').last().type('fgpassword');
    cy.get('button').as('loginButton').click();
    cy.wait(250);
    cy.get('input').eq(2).type('newpass');
    cy.get('input').eq(3).type('newpass');
    cy.get('button').last().as('codeButton').click();
    cy.wait(250);
    cy.getCookie('refreshToken').should('have.property', 'value', 'passExp-refresh-token');
  });
});

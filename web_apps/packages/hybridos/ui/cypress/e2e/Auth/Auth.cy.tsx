import { mfaMock, mfaMockInvalidCode } from 'src/mocks/pages/Auth/MFA.mock'
import { passExpMock, passExpMockOldPassword } from 'src/mocks/pages/Auth/PassExp.mock'
import { loginMfaMock, loginMock, loginMockUnauthorized, loginPassExpMock } from 'src/mocks/pages/Auth/Login.mock'

// TODO: Add ids to components / elements
// TODO: Use better waits after submit actions.
describe('<WebUILogin />', () => {
  beforeEach(() => {
    cy.visit('/')
  })

  it('SUCCESS: Basic Login', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(loginMock)
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.getCookie('refreshToken').should('have.property', 'value', 'fake-flexgen-token')
  })

  it('FAIL: Invalid username.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(loginMockUnauthorized)
    })

    cy.get('input').first().type('badusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.getCookie('refreshToken').should('not.exist')
  })

  it('FAIL: Invalid password.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(loginMockUnauthorized)
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.getCookie('refreshToken').should('not.exist')
  })

  it('SUCCESS: Log in with MFA.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginMfaMock,
        mfaMock,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('input').last().type('1234')
    cy.get('button').last().as('codeButton').click()
    cy.wait(250)
    cy.getCookie('refreshToken').should('have.property', 'value', 'mfa-refresh-token')
  })

  it('FAIL: Log in with MFA, No code entered.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginMfaMock,
        mfaMock,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('button').last().as('codeButton').click()
    cy.wait(250)
    cy.contains('Invalid input. Ensure code field is not empty.').should('exist')
    cy.getCookie('refreshToken').should('not.exist')
  })

  it('FAIL: Log in with MFA, Incorrect Code.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginMfaMock,
        mfaMockInvalidCode,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('input').last().type('1234')
    cy.get('button').last().as('codeButton').click()
    cy.wait(250)
    cy.contains('Incorrect TOTP Code').should('exist')
    cy.getCookie('refreshToken').should('not.exist')
  })

  it('SUCCESS: Log in with Pass Exp.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginPassExpMock,
        passExpMock,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('input').eq(2).type('newpass')
    cy.get('input').eq(3).type('newpass')
    cy.get('button').last().as('codeButton').click()
    cy.wait(250)
    cy.getCookie('refreshToken').should('have.property', 'value', 'passExp-refresh-token')
  })

  it('FAIL: Log in with Pass Exp, passwords to not match.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginPassExpMock,
        passExpMock,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('input').eq(2).type('newpass-one')
    cy.get('input').eq(3).type('newpass-two')
    cy.get('button').last().click()
    cy.wait(250)
    cy.contains('Invalid input. Ensure passwords are not empty and do not match each other.').should('exist')
    cy.getCookie('refreshToken').should('not.exist')
  })

  it('FAIL: Log in with Pass Exp, Invalid passwords.', () => {
    cy.window().then((window) => {
      const { worker } = window.msw
      worker.use(
        loginPassExpMock,
        passExpMockOldPassword,
      )
    })

    cy.get('input').first().type('fgusername')
    cy.get('input').last().type('fgpassword')
    cy.get('button').as('loginButton').click()
    cy.wait(250)
    cy.get('input').eq(2).type('newpass123!')
    cy.get('input').eq(3).type('newpass123!')
    cy.get('button').last().click()
    cy.wait(250)
    cy.contains('New password must not match current password').should('exist')
    cy.getCookie('refreshToken').should('not.exist')
  })
})

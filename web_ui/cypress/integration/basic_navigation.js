describe('Access HybridOS instance running on 001 or localhost', () => {
    it('Accesses HybridOS', () => {
        cy.visit('http://localhost/');
        // cy.visit('https://192.168.6.26');
        cy.get('body');
        cy.title().should('include', 'HybridOS');
    });
});

describe('Go to Site in sidebar', () => {
    it('Goes to Site', () => {
        cy.get('[href="/site"]').contains('Site').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Features in sidebar', () => {
    it('Goes to Features', () => {
        cy.get('[href="/features"]').contains('Features').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Events in sidebar', () => {
    it('Goes to Events', () => {
        cy.get('[href="/events"]').contains('Events').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Storage in sidebar', () => {
    it('Goes to Storage', () => {
        cy.get('[href="/assets/ess"]').contains('Storage').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Generator in sidebar', () => {
    it('Goes to Generator', () => {
        cy.get('[href="/assets/generators"]').contains('Generator').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Solar in sidebar', () => {
    it('Goes to Solar', () => {
        cy.get('[href="/assets/solar"]').contains('Solar').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Feeders in sidebar', () => {
    it('Goes to Feeders', () => {
        cy.get('[href="/assets/feeders"]').contains('Feeders').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Login to Inspector', () => {
    it('Logs in to Inspector', () => {
        cy.get('body').type('{alt}', { release: false })
        cy.get('[data-cy=flexgen_logo]').click();
        cy.get('[data-cy=password_input]').should('be.visible').type('sesame');
    });
});

describe('Go to FIMS in sidebar', () => {
    it('Goes to FIMS', () => {
        cy.get('[href="/inspector/fims"]').contains('FIMS').click();
        cy.get('body').contains('FIMS Send and Listen');
        // makes sure the page has loaded
    });
});

describe('Select "Footer" in sidebar Throughput Display', () => {
    it('Selects Footer', () => {
        cy.get('[data-cy=footer]').contains('Footer').click();
        cy.get('body').contains('Updates and FIMS messages');
        // makes sure the page has loaded
    });
});

describe('Select "Console" in sidebar Throughput Display', () => {
    it('Selects Console', () => {
        cy.get('[data-cy=console]').contains('Console').click();
    });
});

describe('Select "Off" in sidebar Throughput Display', () => {
    it('Selects Off', () => {
        cy.get('[data-cy=off]').contains('Off').click();
    });
});

describe('Toggle sidebar "drawer" to closed', () => {
    it('Toggles drawer closed', () => {
        cy.get('[data-cy=chevron_left_icon]').click();
    });
});

describe('Toggle sidebar "drawer" to open', () => {
    it('Toggles drawer open', () => {
        cy.get('[data-cy=hamburger_icon]').click();
    });
});

describe('Go to Manage Site in Dashboard', () => {
    it('Goes to Manage Site', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_site]').contains('Manage Site').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Manage Features in Dashboard', () => {
    it('Goes to Manage Features', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_features]').contains('Manage Features').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Manage Storage in Dashboard', () => {
    it('Goes to Manage Storage', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_storage]').contains('Manage Storage').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Manage Generators in Dashboard', () => {
    it('Goes to Manage Generators', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_generators]').contains('Manage Generators').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Manage Solar in Dashboard', () => {
    it('Goes to Manage Solar', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_solar]').contains('Manage Solar').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Go to Manage Feeders in Dashboard', () => {
    it('Goes to Manage Feeders', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('[data-cy=summaryCard_manage_feeders]').contains('Manage Feeders').click({ force: true });
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

// we do the following out of order (should be Dashboard, Site, Features...)
// because the page loads on Dashboard so we need a chance to actively select it
describe('Go to Dashboard in sidebar', () => {
    it('Goes to Dashboard', () => {
        cy.get('[href="/dashboard"]').contains('Dashboard').click();
        cy.get('body').contains('Status');
        // makes sure the page has loaded
    });
});

describe('Logout of Inspector', () => {
    it('Logged out of Inspector', () => {
        cy.get('[data-cy=flexgen_logo]').click();
        cy.get('[data-cy=layout_drawer]').should('not.contain', 'Inspector');
        cy.get('[data-cy=layout_drawer]').should('not.contain', 'FIMS');
    });
});

describe('Clear local and session storage', () => {
    it('clears storage', () => {
        cy.clearLocalStorage();
        sessionStorage.clear();
        cy.get('.Layout-content-12').scrollTo('top')
    });
});



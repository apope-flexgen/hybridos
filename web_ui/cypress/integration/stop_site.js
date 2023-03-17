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
    });
});

describe('Stop site - should FAIL if not "Running"', () => {
    it('Stops site', () => {
        cy.get('[data-cy=site_status]').contains('Running')
            .then(() => {
                cy.get('[data-cy=sliderControl_local_disable_flag]').click();
                cy.get('[data-cy=confirmChanges_local_disable_flag]').click();
                cy.wait(5000)
                cy.get('[data-cy=site_status]').invoke('text').should('not.contain', 'Running');
            });
    });
});


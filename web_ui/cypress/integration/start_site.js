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

describe('Start site - should FAIL if site is already "Running", or already in Local Control, or Disable Site is "true"', () => {
    it('Starts site', () => {
        cy.get('[data-cy=site_status]').invoke('text').should('not.contain', 'Running')
            .then(() => {
                cy.get('[data-cy=sliderControl_local_remote_source_flag]').its('0.control.checked').should('eq', false)
                    .then(() => {
                        cy.get('[data-cy=sliderControl_local_remote_source_flag]').click();
                        cy.get('[data-cy=confirmChanges_local_remote_source_flag]').click();
                        cy.wait(1500); // if SliderControl hasn't received a response in 1000 ms, the value
                        // will return to what it was before.
                        cy.get('[data-cy=sliderControl_local_remote_source_flag]').its('0.control.checked').should('eq', true)
                            .then((result) => {
                                cy.get('[data-cy=buttonControl_local_enable_flag]').click();
                                cy.get('[data-cy=confirmChanges_local_enable_flag]').click();
                                cy.wait(5000)
                                cy.get('[data-cy=site_status]').contains('Running');
                            });
                    });
            });
    });
});


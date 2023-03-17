import React, { Component } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate, useRoutes } from 'react-router-dom';
import UIConfig from './app/component/UIConfig';
import LoginPage from './app/view/LoginPage';
import Layout from './app/component/Layout';
import Dashboard from './app/view/Dashboard';
import FleetManagerDashboard from './app/view/FleetManagerDashboard';
import Site from './app/view/Site';
import Features from './app/view/Features';
import EventsPage from './app/view/Events';
import ComponentsPage from './app/view/ComponentsPage';
import UserAdministration from './app/view/UserAdministration';
import SiteAdministration from './app/view/SiteAdministration';
import InspectorPage from './app/view/InspectorPage';
import AssetsPageNew from './app/view/AssetsPageNew';
import AssetsPage from './app/view/AssetsPage';
import Scheduler from './app/view/Scheduler';
import SchedulerConfig from './app/view/SchedulerConfig';
import VariableOverride from './app/view/VariableOverride'

import {
    ESS_PAGE_PATH,
    GENERATOR_PAGE_PATH,
    SOLAR_PAGE_PATH,
    FEEDERS_PAGE_PATH,
    CONTROL_CABINET_PAGE_PATH,
    USER_ADMINISTRATION_PAGE_PATH,
    SITE_ADMINISTRATION_PAGE_PATH,
    COMPONENTS_PAGE_PATH,
    FIMS_PAGE_PATH,
    BMS_PAGE_PATH,
    PCS_PAGE_PATH,
    SCHEDULER_PAGE_PATH,
    DBI_DOWNLOAD_PAGE_PATH,
    SCHEDULER_CONFIGURATION_PATH,
    WEBUI_CONFIG_PATH,
    VAR_OVERRIDE_PAGE_PATH,
    getDataForURI
} from './AppConfig';
import { siteConfiguration } from './AppAuth';
import { tokenLogin, socket_auth } from './AppAuth';
import { resetWarningCache } from 'prop-types';
/**
 * Component for routing pages
 * @extends Component
 */
class AppRoutes extends Component {
    /**
     * Initializes as not logged in to display login page
     */
    constructor() {
        super();
        this.state = {
            isLoggedIn: false,
            configuredLayout: false,
        };
        this.setStateFromChild = this.setStateFromChild.bind(this);
    }

    /**
     * Sets login state from children
     * @param {*} key unused
     * @param {boolean} value true if successful login
     */
    setStateFromChild(key, value) {
        this.setState({ isLoggedIn: value });
    }
    // validate jwt cookie if present and return role
    componentDidMount(){
        fetch('/api/authenticate-user-token', {
            credentials: 'include',
        })
            .then((res) => res.json())
            .then((res) => {
                if (res.role) {
                    tokenLogin(res.user, res.role, ()=> this.setStateFromChild('isLoggedIn', true));
                }   
            });
    }

    componentDidUpdate(prevProps, prevState) {
        // Wait for login confirmation to request drawer layout
        if (this.state.isLoggedIn && !prevState.isLoggedIn) {
            getDataForURI('/dbi/ui_config/show_documents')
            .then((res) => res.json())
            .then((res) => {
                if (res.body.includes('layout')) {
                    getDataForURI('/dbi/ui_config/layout')
                        .then((res) => res.json())
                        .then((res) => {
                            this.setState({ layout: res.body.data });
                        })
                }
            })
        }
    }
    
    componentWillUnmount(){
        // destroy the socket after a session is over
        socket_auth.disconnect();
    }
    
    // BAND-AID FOR REACT-ROUTER V6
    // now contained within <Route element>s
    // https://stackoverflow.com/q/71457920
    /**
     * Checks if page should display as grid type
     * @param {string} type grid or linear
     */
    checkGrid = (page, props) => {
        if (siteConfiguration.setGrid && siteConfiguration.setGrid.includes(page)) {
            return <AssetsPageNew {...props} page={page.toUpperCase()} />
        }

        return <AssetsPage {...props} type={page} pageType='linear'/>
    }

    render() {
        if (this.state.isLoggedIn) {
            return (
                <>
                    <Router>
                        <Layout
                            layout={this.state.layout}
                        >
                            <Routes>
                                <Route path="/" element={<Navigate replace to="/dashboard" />} />
                                <Route
                                    path={WEBUI_CONFIG_PATH}
                                    element={<UIConfig type="ConfigPage" />}
                                />
                                <Route path="/dashboard" element={siteConfiguration.includeFleetManagerDashboard ? <FleetManagerDashboard /> : <Dashboard />} />
                                <Route path="/features" element={<Features />} />
                                <Route path="/site" element={<Site />} />
                                <Route path="/events" element={<EventsPage />} />
                                <Route
                                    path={ESS_PAGE_PATH}
                                    element={(siteConfiguration.setGrid && siteConfiguration.setGrid.includes('ess'))
                                        ? (<AssetsPageNew page="ESS" />)
                                        : (<AssetsPage type="ess" pageType="linear" />)
                                    }
                                />
                                <Route
                                    path={GENERATOR_PAGE_PATH}
                                    element={(siteConfiguration.setGrid && siteConfiguration.setGrid.includes('generators'))
                                        ? (<AssetsPageNew page="GENERATORS" />)
                                        : (<AssetsPage type="generators" pageType="linear" />)
                                    }
                                />
                                <Route
                                    path={SOLAR_PAGE_PATH}
                                    element={(siteConfiguration.setGrid && siteConfiguration.setGrid.includes('solar'))
                                        ? (<AssetsPageNew page="SOLAR" />)
                                        : (<AssetsPage type="solar" pageType="linear" />)
                                    }
                                />
                                <Route
                                    path={FEEDERS_PAGE_PATH}
                                    element={(siteConfiguration.setGrid && siteConfiguration.setGrid.includes('feeders'))
                                        ? (<AssetsPageNew page="FEEDERS" />)
                                        : (<AssetsPage type="feeders" pageType="linear" />)
                                    }
                                />
                                {this.state.layout && this.state.layout.map((tab) => (
                                    <Route
                                        path={`/${tab.info.key}`}
                                        element={<AssetsPageNew page={tab.info.name} pageKey={tab.info.key} key={tab.info.key}/>}
                                        key={tab.info.key}
                                    />
                                ))}
                                <Route
                                    path={SCHEDULER_PAGE_PATH}
                                    element={<Scheduler type="scheduler" />}
                                />
                                <Route
                                    path={USER_ADMINISTRATION_PAGE_PATH}
                                    element={<UserAdministration /> }
                                />
                                <Route
                                    path={VAR_OVERRIDE_PAGE_PATH}
                                    element={<VariableOverride />}
                                />
                                <Route
                                    path={SITE_ADMINISTRATION_PAGE_PATH}
                                    element={<SiteAdministration />}
                                />
                                <Route
                                    path={CONTROL_CABINET_PAGE_PATH}
                                    element={<ComponentsPage type="control_cabinet" />}
                                />
                                <Route
                                    path={COMPONENTS_PAGE_PATH}
                                    element={<InspectorPage type="components" />}
                                />
                                <Route
                                    path={FIMS_PAGE_PATH}
                                    element={<InspectorPage type="fims" />}
                                />
                                <Route
                                    path={DBI_DOWNLOAD_PAGE_PATH}
                                    element={<InspectorPage type="dbiDownload" />}
                                />
                                <Route
                                    path={SCHEDULER_CONFIGURATION_PATH}
                                    element={<SchedulerConfig type="SchedulerConfig" />}
                                />
                            </Routes>
                        </Layout>
                    </Router>
                </>
            );
        }
        return (<LoginPage setStateFromChild={this.setStateFromChild} />);
    }
}

export default AppRoutes;

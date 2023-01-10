/* eslint-disable no-unused-vars */
const postAssert = {
    method: 'post',
    uri: '/events',
    replyto: '/assets/ess/ess1/epo_event',
    body: {
        source: '/assets/ess/ess1',
        message: 'Energy Storage System 1 has shut down due to EPO',
        severity: 4,
        assert: true,
    },
};

const setReplyAssert = {
    method: 'set',
    uri: '/assets/ess/ess1/epo_event',
    replyto: null,
    body: {
        _id: '5bd0a1d6a383fba2489a6b3c',
        timestamp: '2018-10-24T16:47:33.820Z',
        source: '/assets/ess/ess1',
        message: 'Energy Storage System 1 has shut down due to EPO',
        severity: 4,
        assert: true,
        active: true,
    },
};

const getActive = {
    method: 'get',
    uri: '/events',
    replyto: '/ui/events',
    body: { // all query parameters are optional
        severity: 1, // greater than or equal, default 1
        after: '2018-10-24T16:40:00.000Z', // Time range filtering
        before: '2018-10-24T16:50:00.000Z', // None, one or both can be used
        source: '/assets/ess', // Like subscribes, will give you entries
        // from deeper uris
        active: true,
        limit: 100, // Maximum how many entries will be returned, will help query
        // performance when database gets deep. Default 100
        page: 1, // Working together with "limit", you can dig farther back
        // Entries are returned in descending timestamp order, i.e.
        // page 1 (default) has the newest entries, subsequent pages are older
    },
};

// const setActive = {
//   "method": "set",
//   "uri": "/ui/events",
//   "replyto": null,
//   "body": {
//     "data": [
//       {
//         "_id": "5bd0a1d6a383fba2489a6b3c",
//         "timestamp": "2018-10-24T16:47:33.820Z",
//         "source": "/assets/ess/ess1",
//         "message": "Energy Storage System 1 has shut down due to EPO",
//         "severity": 4,
//         "assert": true,
//         "active": true
//       },
//       {
//         "_id": "5bd0a1d6a383fba2489a647x",
//         "timestamp": "2018-10-24T16:41:31.857Z",
//         "source": "/assets/ess/ess2",
//         "message": "Energy Storage System 2 has shut down due to EPO",
//         "severity": 4,
//         "assert": true,
//         "active": true
//       }
//     ]
//   }
// };

const postClear = {
    method: 'post',
    uri: '/events',
    replyto: '/assets/ess/ess1/epo_event',
    body: {
        source: '/assets/ess/ess1',
        message: 'Energy Storage System 1 EPO has been cleared',
        severity: 4,
        clear: true,
        assertId: '5bd0a1d6a383fba2489a6b3c',
    },
};

const setReplyClear = {
    method: 'set',
    uri: '/assets/ess/ess1/epo_event',
    replyto: null,
    body: {
        _id: '5bd0a328accf630aa5808dc6',
        timestamp: '2018-10-24T16:51:53.347Z',
        source: '/assets/ess/ess1',
        message: 'Energy Storage System 1 EPO has been cleared',
        severity: 4,
        clear: true,
        assertId: '5bd0a1d6a383fba2489a6b3c',
    },
};

const getAll = {
    method: 'get',
    uri: '/events',
    replyto: '/ui/events',
    body: {
        severity: 1,
        after: '2018-10-24T16:40:00.000Z',
        before: '2018-10-24T16:52:00.000Z',
        source: '/assets/ess',
        limit: 100,
        page: 1,
    },
};

const setActive = {
    method: 'set',
    uri: '/ui/events',
    replyto: null,
    body: {
        data: [
            {
                _id: '5bd0a328accf630aa5808dc6',
                timestamp: '2018-10-24T16:51:53.347Z',
                source: '/assets/ess/ess1',
                message: 'Energy Storage System 1 EPO has been cleared',
                severity: 4,
                clear: true,
                assertId: '5bd0a1d6a383fba2489a6b3c',
            },
            {
                _id: '5bd0a1d6a383fba2489a6b3c',
                timestamp: '2018-10-24T16:47:33.820Z',
                source: '/assets/ess/ess1',
                message: 'Energy Storage System 1 has shut down due to EPO',
                severity: 4,
                assert: true,
                active: false,
            },
            {
                _id: '5bd0a1d6a383fba2489a647x',
                timestamp: '2018-10-24T16:41:31.857Z',
                source: '/assets/ess/ess2',
                message: 'Energy Storage System 2 has shut down due to EPO',
                severity: 4,
                assert: true,
                active: true,
            },
        ],
    },
};

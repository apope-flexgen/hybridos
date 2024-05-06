const internalServerError = {
    success: false,
    message: 'Internal Server Error',
};
const invalidJsonError = {
    success: false,
    message: 'Supplied body is invalid JSON',
};
function formatList(l) {
    return `[${(l || []).map((x) => `'${x}'`).join(',')}]`;
}
/**
 * Validate a msg object alongside required keys
 * @param msg A message which contains a body
 * @param config { requiredGroups: [ ["id"], ["field1", "field2"], optional: []]}
 * @returns "{ success: boolean, message: string }"
 */
function validateAlert(msg, config) {
    try {
        if (Object.keys(msg).includes('body')) {
            if (['string', 'number'].includes(typeof msg.body)) {
                JSON.parse(msg.body);
            } else if (typeof msg.body === 'object') {
                JSON.stringify(msg.body);
            } else {
                return internalServerError;
            }
        }
    } catch (err) {
        return invalidJsonError;
    }
    const unrecognized = [];
    for (const key of Object.keys(msg.body || {})) {
        let found = false;
        for (const [groupIndex, group] of config.requiredGroups.entries()) {
            const index = group.indexOf(key);
            if (index >= 0) {
                config.requiredGroups[groupIndex].splice(index, 1);
                found = true;
            }
        }
        if (!found && !config.optional.includes(key)) {
            unrecognized.push(key);
        }
    }
    // if we find an empty requirement option set
    if (config.requiredGroups.length === 0 || config.requiredGroups.some((g) => g.length === 0)) {
        return { success: true, message: null };
    }
    return {
        success: false,
        message: `Invalid request - missing required keys: ${formatList(config.requiredGroups[0])} ${unrecognized.length ? `, unrecognized keys: ${formatList(unrecognized)}` : ''}`,
    };
}

module.exports = {
    validateAlert,
};

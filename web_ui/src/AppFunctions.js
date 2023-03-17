import moment from 'moment-timezone'

/**
 * Checks if a day is a DST switch day.
 * @param {boolean} isToday - true if checking if today (according to timeline in scheduler) is a DST day, false for tomorrow
 * @param {boolean} springForward - true if checking if the day specified is a spring forward day, false for fall back
 * @param {string} timezone - timezone string like "America/Chicago"
*/
export function isDayDST(isToday, springForward, timezone) {
    let currentTime = new Date();    
    const convertTime = moment().tz(timezone);
    let isDSTToday = moment().tz(timezone).startOf('day').isDST(); //add 1

    let isDSTTomorrow = moment().tz(timezone).add(1, 'days').startOf('day').isDST() //add 2

    let isDSTTomorrowTomorrow = moment().tz(timezone).add(2, 'days').startOf('day').isDST() //add 3


    if (isToday) {
        if (springForward) {
            if (!isDSTToday && isDSTTomorrow) {
                return true;
            } else {
                return false;
            }
        } else {
            if (isDSTToday && !isDSTTomorrow) {
                return true;
            } else {
                return false;
            }
        }
    }

    if (!isToday) {
        if (springForward) {
            if (!isDSTTomorrow && isDSTTomorrowTomorrow) {
                return true;
            } else {
                return false;
            }
        } else {
            if (isDSTTomorrow && !isDSTTomorrowTomorrow) {
                return true;
            } else {
                return false;
            }
        }
    }

    


}
import { Observable, takeWhile } from 'rxjs'

export const socketObservableCleanup = (socket: WebSocket) => {
    return <T>(source: Observable<T>): Observable<T> => {
        return source.pipe(takeWhile(() => socket.readyState <= socket.CLOSING, false))
    }
}

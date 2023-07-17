import { Catch, ArgumentsHost, ExceptionFilter } from '@nestjs/common';
import { LoggingService } from 'src/logging/logging.service';
import { LogText } from 'src/logging/log_text/log-text';

@Catch()
export class AppExceptionsFilter implements ExceptionFilter {
    constructor(private readonly loggingService: LoggingService) { }
    catch(exception: any, host: ArgumentsHost) {
        const context = host.getType();
        let target = '';
        if (context === 'http') {
            target = host.switchToHttp().getRequest().url;
        } else if (context === 'ws') {
            // 'ws' context type has no url. getData() is a 
            // meaningful way to determine the target resource.
            target = host.switchToWs().getData();
        }
        const log: LogText = {
            message: exception.message,
            stack: exception.stack,
            url: target,
        }
        this.loggingService.error(log, context)
    }
}

import { CanActivate, ExecutionContext, Injectable } from '@nestjs/common';
import { Reflector } from '@nestjs/core';

@Injectable()
export class RolesGuard implements CanActivate {
  constructor(private reflector: Reflector) {}
  async canActivate(context: ExecutionContext): Promise<boolean> {
    const roles = this.reflector.get<string[]>('roles', context.getHandler());
    if (!roles) {
      return true;
    }

    const request = context.switchToHttp().getRequest();
    const user = request.user;
    return await this.matchRoles(roles, user.role);
  }
  async matchRoles(roles: string[], userRole: string): Promise<boolean> {
    return new Promise((resolve) => {
      roles.forEach((role) => {
        if (role === userRole) {
          resolve(true);
        }
      });
      resolve(false);
    });
  }
}

#pragma once
struct GcomSystem;
struct DNP3Dependencies;

bool init_tmw(GcomSystem &sys);
void setupTMWIOConfig(GcomSystem &sys);
bool openTMWServerChannel(GcomSystem &sys);
bool openTMWServerSession(GcomSystem &sys);
bool openTMWClientChannel(GcomSystem &sys);
bool openTMWClientSession(GcomSystem &sys);
void shutdown_tmw(DNP3Dependencies *dnp3_sys);
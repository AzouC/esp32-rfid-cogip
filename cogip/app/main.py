from fastapi import FastAPI, Depends, HTTPException
from fastapi.responses import FileResponse
from pydantic import BaseModel
from sqlalchemy import select
from sqlalchemy.orm import Session

from .database import Base, engine, get_db
from .models import User, Badge, Zone, AccessRight, AccessLog

Base.metadata.create_all(bind=engine)
app = FastAPI(title="Pointeuse COGIP")


# ---------- Schémas ----------
class UserIn(BaseModel):
    nom: str
    prenom: str
    service: str = ""

class BadgeIn(BaseModel):
    nuid: str
    user_id: int

class ZoneIn(BaseModel):
    nom: str

class RightIn(BaseModel):
    user_id: int
    zone_id: int

class ScanIn(BaseModel):
    nuid: str
    zone: str = "Entrée principale"


# ---------- Administration ----------
@app.post("/api/users")
def create_user(data: UserIn, db: Session = Depends(get_db)):
    user = User(**data.model_dump())
    db.add(user); db.commit(); db.refresh(user)
    return {"id": user.id}

@app.post("/api/badges")
def create_badge(data: BadgeIn, db: Session = Depends(get_db)):
    badge = Badge(nuid=data.nuid.upper(), user_id=data.user_id)
    db.add(badge); db.commit(); db.refresh(badge)
    return {"id": badge.id}

@app.post("/api/zones")
def create_zone(data: ZoneIn, db: Session = Depends(get_db)):
    zone = Zone(nom=data.nom)
    db.add(zone); db.commit(); db.refresh(zone)
    return {"id": zone.id}

@app.post("/api/rights")
def grant_right(data: RightIn, db: Session = Depends(get_db)):
    db.add(AccessRight(**data.model_dump())); db.commit()
    return {"status": "ok"}


# ---------- Coeur du système ----------
@app.post("/api/scan")
def scan(data: ScanIn, db: Session = Depends(get_db)):
    nuid = data.nuid.upper()
    badge = db.scalar(select(Badge).where(Badge.nuid == nuid, Badge.actif.is_(True)))
    utilisateur, autorise = "Inconnu", False

    if badge:
        utilisateur = f"{badge.user.prenom} {badge.user.nom}"
        zone = db.scalar(select(Zone).where(Zone.nom == data.zone))
        if zone:
            droit = db.scalar(select(AccessRight).where(
                AccessRight.user_id == badge.user_id,
                AccessRight.zone_id == zone.id))
            autorise = droit is not None

    db.add(AccessLog(nuid=nuid, zone=data.zone,
                     utilisateur=utilisateur, autorise=autorise))
    db.commit()
    return {"nuid": nuid, "utilisateur": utilisateur, "autorise": autorise}


@app.get("/api/last")
def last_scan(db: Session = Depends(get_db)):
    log = db.scalar(select(AccessLog).order_by(AccessLog.id.desc()).limit(1))
    if not log:
        raise HTTPException(404, "Aucun passage enregistré")
    return {"horodatage": log.horodatage.isoformat(timespec="seconds"),
            "nuid": log.nuid, "zone": log.zone,
            "utilisateur": log.utilisateur, "autorise": log.autorise}


@app.get("/")
def index():
    return FileResponse("app/static/index.html")

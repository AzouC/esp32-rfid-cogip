from datetime import datetime
from sqlalchemy import ForeignKey, String, Boolean, DateTime, UniqueConstraint
from sqlalchemy.orm import Mapped, mapped_column, relationship
from .database import Base


class User(Base):
    __tablename__ = "users"
    id: Mapped[int] = mapped_column(primary_key=True)
    nom: Mapped[str] = mapped_column(String(80))
    prenom: Mapped[str] = mapped_column(String(80))
    service: Mapped[str] = mapped_column(String(80), default="")
    badges: Mapped[list["Badge"]] = relationship(back_populates="user")


class Badge(Base):
    __tablename__ = "badges"
    id: Mapped[int] = mapped_column(primary_key=True)
    nuid: Mapped[str] = mapped_column(String(20), unique=True, index=True)
    actif: Mapped[bool] = mapped_column(Boolean, default=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"))
    user: Mapped[User] = relationship(back_populates="badges")


class Zone(Base):
    __tablename__ = "zones"
    id: Mapped[int] = mapped_column(primary_key=True)
    nom: Mapped[str] = mapped_column(String(80), unique=True)


class AccessRight(Base):
    __tablename__ = "access_rights"
    __table_args__ = (UniqueConstraint("user_id", "zone_id"),)
    id: Mapped[int] = mapped_column(primary_key=True)
    user_id: Mapped[int] = mapped_column(ForeignKey("users.id"))
    zone_id: Mapped[int] = mapped_column(ForeignKey("zones.id"))


class AccessLog(Base):
    __tablename__ = "access_logs"
    id: Mapped[int] = mapped_column(primary_key=True)
    horodatage: Mapped[datetime] = mapped_column(DateTime, default=datetime.now)
    nuid: Mapped[str] = mapped_column(String(20))
    zone: Mapped[str] = mapped_column(String(80))
    utilisateur: Mapped[str] = mapped_column(String(160), default="Inconnu")
    autorise: Mapped[bool] = mapped_column(Boolean)
    
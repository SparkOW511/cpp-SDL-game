#pragma once

#include "Components.hpp"

class AmmoComponent : public Component {
public:
    int currentAmmo;
    int maxAmmo;
    int ammoPerMagazine;

    AmmoComponent(int maxAmmo, int ammoPerMagazine) {
        this->maxAmmo = maxAmmo;
        this->currentAmmo = maxAmmo;
        this->ammoPerMagazine = ammoPerMagazine;
    }

    bool canShoot() {
        return currentAmmo > 0;
    }

    void shoot() {
        if (currentAmmo > 0) {
            currentAmmo--;
        }
    }

    void addAmmo() {
        currentAmmo = std::min(maxAmmo, currentAmmo + ammoPerMagazine);
    }
}; 
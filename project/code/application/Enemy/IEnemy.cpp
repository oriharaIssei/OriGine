#include "IEnemy.h"

#include "object3d/Object3d.h"

IEnemy::IEnemy() {
    // Constructor implementation
}

IEnemy::~IEnemy() {
    // Destructor implementation
}

void IEnemy::Draw() {
    drawObject3d_->Draw();
}

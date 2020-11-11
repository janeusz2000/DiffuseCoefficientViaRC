#include "constants.h"
#include "main/simulator.h"
#include "gtest/gtest.h"

#include <utility>

using constants::kPi;
using core::Ray;
using core::RayHitData;
using core::Vec3;

const int kSkipNumCollectors = 37;
const float kSkipFrequency = 1000;
const Vec3 kVecZero(0, 0, 0);
const Vec3 kVecUp(0, 0, 1);
const Vec3 kVecX(1, 0, 0);
const Vec3 kVecY(0, 1, 0);

float deg2rad(float deg) { return 2 * kPi * deg / 360; }

// TODO: add collision point check for every ray hit

class FakeModel : public ModelInterface {

public:
  explicit FakeModel(bool empty) : empty_(empty){};
  std::vector<objects::TriangleObj *> triangles() const override { return {}; }
  float height() const override { return 0; }
  float sideSize() const override { return 0; }
  bool empty() const override { return empty_; }

private:
  bool empty_;
};

class EnergyCollectorTest : public ::testing::Test {
protected:
  // performs ray hit at at given energy collectors, |hitData| is modified to
  // hold information where ray hit energyCollector. Returns true if hit
  // occurred, false when there was no hit
  [[nodiscard]] bool performHitCollector(
      const std::vector<std::unique_ptr<objects::EnergyCollector>>
          &energyCollectors,
      const Ray &ray, RayHitData *hitData) {

    for (const auto &collector : energyCollectors) {
      if (collector->hitObject(ray, kSkipFrequency, hitData)) {
        return true;
      }
    }

    return false;
  }

  void
  printCollectors(const std::vector<std::unique_ptr<objects::EnergyCollector>>
                      &energyCollectors) const {
    for (const auto &collector : energyCollectors) {
      std::cout << "Origin: " << collector->getOrigin() << "\n"
                << "Radius: " << collector->getRadius() << "\n";
    }
  }

  std::pair<objects::EnergyCollector, objects::EnergyCollector>
  topCollectorsXAxis(
      const std::vector<std::unique_ptr<objects::EnergyCollector>>
          &energyCollectors) {
    auto outputCollectors =
        std::make_pair(objects::EnergyCollector(Vec3(0, 0, 0), 1),
                       objects::EnergyCollector(Vec3(0, 0, 0), 1));
    for (const auto &collector : energyCollectors) {
      if (collector->getOrigin().z() >=
              outputCollectors.first.getOrigin().z() &&
          collector->getOrigin().x() ==
              outputCollectors.first.getOrigin().x()) {
        outputCollectors.second = outputCollectors.first;
        outputCollectors.first = *(collector.get());
      }
    }
    return outputCollectors;
  }
};
TEST_F(EnergyCollectorTest, ThrowExceptionWhenInvalidNumCollector) {

  const FakeModel nonEmptyModel(false);

  const int invalidNumCollectors = 38;
  try {
    buildCollectors(nonEmptyModel, invalidNumCollectors);
    FAIL() << "Expected throw std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    EXPECT_STREQ("numCollectors or numCollectors-1 has to be "
                 "divisible by 4, got numCollectors = 38",
                 e.what());
  } catch (...) {
    FAIL() << "caught wrong exception";
  }

  const int numCollectorLessThenOne = 3;
  try {
    buildCollectors(nonEmptyModel, numCollectorLessThenOne);
    FAIL() << "Expected throw std::invalid_argument";
  } catch (const std::invalid_argument &e) {
    EXPECT_STREQ("numCollectors: 3 is less then 4", e.what());
  } catch (...) {
    FAIL() << "Caught wrong exception";
  }

  // Test case when numCollector - 1 % 4 = 0
  const int validNumCollectorCase1 = 37;
  EXPECT_NO_THROW(buildCollectors(nonEmptyModel, validNumCollectorCase1));

  // Test case when numCollector % 4 = 0
  const int validNumCollectorCase2 = 36;
  EXPECT_NO_THROW(buildCollectors(nonEmptyModel, validNumCollectorCase2));
}

TEST_F(EnergyCollectorTest, NotEvenNumOfEnergyCollectorTest) {
  const FakeModel nonEmptyModel(false);

  const int numCollectors = 37;
  auto energyCollectors = buildCollectors(nonEmptyModel, numCollectors);
  ASSERT_EQ(numCollectors, energyCollectors.size());

  Ray straightUp(kVecZero, kVecUp);
  RayHitData hitData;
  ASSERT_TRUE(performHitCollector(energyCollectors, straightUp, &hitData));

  const float collectorPositionRadius = 4;
  const float collectorAngle = 2 * kPi / (numCollectors - 1);
  const float refCollectorRadius =
      collectorPositionRadius * std::sqrt(2 - 2 * std::cos(collectorAngle));
  Vec3 collisionPointStraightUp =
      Vec3(0, 0, collectorPositionRadius - refCollectorRadius);
  ASSERT_EQ(hitData.collisionPoint(), collisionPointStraightUp);

  Ray straightDown(kVecZero, -kVecUp);
  ASSERT_FALSE(performHitCollector(energyCollectors, straightDown, &hitData))
      << "Collision Point: " << hitData.collisionPoint();

  Ray alongX(kVecZero, kVecX);
  ASSERT_TRUE(performHitCollector(energyCollectors, alongX, &hitData));

  ASSERT_EQ(collectorPositionRadius - refCollectorRadius, hitData.time);

  Ray alongY(kVecZero, kVecY);
  ASSERT_TRUE(performHitCollector(energyCollectors, alongY, &hitData));

  ASSERT_EQ(alongY.at(collectorPositionRadius - refCollectorRadius),
            hitData.collisionPoint());

  Ray atSixty(kVecZero, Vec3(std::cos(deg2rad(60)), 0, std::sin(deg2rad(60))));
  ASSERT_TRUE(performHitCollector(energyCollectors, atSixty, &hitData));

  Ray atSixtyOther(kVecZero,
                   Vec3(-std::cos(deg2rad(60)), 0, std::sin(deg2rad(60))));
  ASSERT_TRUE(performHitCollector(energyCollectors, atSixtyOther, &hitData));

  Ray atSixtyXY(kVecZero, Vec3(std::cos(deg2rad(60)), std::cos(deg2rad(60)),
                               std::sin(deg2rad(60))));
  ASSERT_FALSE(performHitCollector(energyCollectors, atSixtyXY, &hitData))
      << "Collision Point: " << hitData.collisionPoint();

  Ray atSixtyXYOther(kVecZero,
                     Vec3(-std::cos(deg2rad(60)), -std::cos(deg2rad(60)),
                          std::sin(deg2rad(60))));
  ASSERT_FALSE(performHitCollector(energyCollectors, atSixtyXYOther, &hitData))
      << "Collision Point: " << hitData.collisionPoint();
}

TEST_F(EnergyCollectorTest, EvenNumOfEnergyCollectorTest) {
  const FakeModel nonEmptyModel(false);

  const int numCollectors = 36;
  auto energyCollectors = buildCollectors(nonEmptyModel, numCollectors);
  ASSERT_EQ(energyCollectors.size(), numCollectors);

  Ray straightUp(kVecZero, kVecUp);
  RayHitData hitData;
  ASSERT_TRUE(performHitCollector(energyCollectors, straightUp, &hitData));

  const float collectorPositionRadius = 4;
  const float collectorAngle = 2 * kPi / (numCollectors - 2);
  const float refCollectorRadius =
      collectorPositionRadius * std::sqrt(2 - 2 * std::cos(collectorAngle));

  std::pair<objects::EnergyCollector, objects::EnergyCollector> topCol =
      topCollectorsXAxis(energyCollectors);
  // this comes from the fact, two origins of neighborhood collectors and
  // collision point are creates equilateral triangle which side is equal to
  // collector radius. Thats why collision point its just the point between two
  // collectors origin - height of the equilateral triangle.
  Vec3 refCollision =
      (topCol.first.getOrigin() + topCol.second.getOrigin()) / 2 -
      Vec3(0, 0, refCollectorRadius * std::sqrt(3) / 2);
  ASSERT_EQ(refCollision, hitData.collisionPoint());

  Ray straightDown(kVecZero, -kVecUp);
  ASSERT_FALSE(performHitCollector(energyCollectors, straightDown, &hitData))
      << "Collision Point: " << hitData.collisionPoint();

  Ray alongX(kVecZero, kVecX);
  ASSERT_TRUE(performHitCollector(energyCollectors, alongX, &hitData));

  ASSERT_EQ(alongX.at(collectorPositionRadius - refCollectorRadius),
            hitData.collisionPoint());

  Ray alongY(kVecZero, kVecY);
  ASSERT_TRUE(performHitCollector(energyCollectors, alongY, &hitData));

  ASSERT_EQ(alongY.at(collectorPositionRadius - refCollectorRadius),
            hitData.collisionPoint());

  Ray at30(kVecZero, Vec3(std::cos(deg2rad(30)), 0, std::sin(deg2rad(30))));
  ASSERT_TRUE(performHitCollector(energyCollectors, at30, &hitData));

  Ray at30other(kVecZero,
                Vec3(-std::cos(deg2rad(30)), 0, std::sin(deg2rad(30))));
  ASSERT_TRUE(performHitCollector(energyCollectors, at30other, &hitData));

  Ray at30XY(kVecZero, Vec3(std::cos(deg2rad(30)), std::cos(deg2rad(30)),
                            std::sin(deg2rad(60))));
  ASSERT_FALSE(performHitCollector(energyCollectors, at30XY, &hitData))
      << "Collision Point: " << hitData.collisionPoint();

  Ray at30XYOther(kVecZero, Vec3(-std::cos(deg2rad(30)), -std::cos(deg2rad(30)),
                                 std::sin(deg2rad(60))));
  ASSERT_FALSE(performHitCollector(energyCollectors, at30XYOther, &hitData))
      << "Collision Point: " << hitData.collisionPoint();
}

TEST_F(EnergyCollectorTest, NoHoleNextToTheTopCollectorOddNum) {

  const FakeModel nonEmptyModel(false);
  const int numCollectors = 37;

  auto energyCollectors = buildCollectors(nonEmptyModel, numCollectors);
  ASSERT_EQ(energyCollectors.size(), numCollectors);

  const float collectorPositionRadius = 4;
  // this is how previous implementation was caclualating radius of energy
  // collector
  float invalidEnergyCollectorRadius =
      2 * kPi * collectorPositionRadius / static_cast<float>(numCollectors);

  RayHitData hitData;
  Ray previousNotHit1(kVecZero, Vec3(0, 1.01 * invalidEnergyCollectorRadius,
                                     collectorPositionRadius));
  ASSERT_TRUE(performHitCollector(energyCollectors, previousNotHit1, &hitData));

  Ray previousNotHit2(kVecZero, Vec3(0, -1.01 * invalidEnergyCollectorRadius,
                                     collectorPositionRadius));

  ASSERT_TRUE(performHitCollector(energyCollectors, previousNotHit2, &hitData));
}

TEST_F(EnergyCollectorTest, HitRayStraightUpEvenCollectors) {

  const FakeModel nonEmptyModel(false);
  const int numCollectors = 20;

  auto energyCollectors = buildCollectors(nonEmptyModel, numCollectors);
  ASSERT_EQ(energyCollectors.size(), numCollectors);

  RayHitData hitData;
  Ray straightUp(kVecZero, kVecUp);
  ASSERT_TRUE(performHitCollector(energyCollectors, straightUp, &hitData));

  const float collectorPositionRadius = 4;
  const float collectorAngle = 2 * kPi / (numCollectors - 2);
  const float refCollectorRadius =
      collectorPositionRadius * std::sqrt(2 - 2 * std::cos(collectorAngle));

  std::pair<objects::EnergyCollector, objects::EnergyCollector> topCol =
      topCollectorsXAxis(energyCollectors);
  // See EvenNumOfEnergyCollectorTest for explanation
  Vec3 refCollision =
      (topCol.first.getOrigin() + topCol.second.getOrigin()) / 2 -
      Vec3(0, 0, refCollectorRadius * std::sqrt(3) / 2);

  ASSERT_EQ(refCollision, hitData.collisionPoint());
}

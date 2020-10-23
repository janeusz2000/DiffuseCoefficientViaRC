#include "objects.h"
#include <sstream>

namespace objects
{

    void Object::setOrigin(const core::Vec3 &origin)
    {
        origin_ = origin;
    }
    core::Vec3 Object::getOrigin() const
    {
        return origin_;
    }

    core::Vec3 Sphere::normal(const core::Vec3 &surfacePoint) const
    {
        return (surfacePoint - getOrigin()).normalize();
    }

    Sphere::Sphere(const core::Vec3 &origin, float rad) {
      this->setOrigin(origin);
      radius_ = rad;
    }

    bool Sphere::hitObject(const core::Ray &ray, float freq, core::RayHitData *hitData)
    {
        core::Vec3 rVec3 = ray.getOrigin() - this->getOrigin();

        // this calculates variables that are neccesary to calculate times at which ray hits the object
        float beta = 2 * rVec3.scalarProduct(ray.getDirection());
        float gamma = rVec3.scalarProduct(rVec3) - radius_ * radius_;
        float discriminant = beta * beta - 4 * gamma;

        // when hit of the sphere never occur:
        if (discriminant < 0) 
        {
            return false;
        }

        float temp = std::sqrt(discriminant);
        // times that ray hits sphere at:
        float time1 = (-beta - temp) / 2;
        float time2 = (-beta + temp) / 2;

        // Ray inside sphere. kAccurracy is for the case that ray is on the edge
        // of the sphere. time < 0 - hit behind ray, time > 0 - hit front of ray
        if (time1 < -constants::kAccuracy && time2 > constants::kAccuracy) 
        {
            core::Vec3 collision = ray.at(time2);
            *hitData = core::RayHitData(time2, normal(collision), ray, freq);
            return true;
        }
        // ray in front of the sphere. kAccuracy if for the edge case
        else if (time1 > constants::kAccuracy && time2 > constants::kAccuracy) 
        {
            float time = std::min(time1, time2);
            core::Vec3 collision = ray.at(time);
            *hitData = core::RayHitData(time, normal(collision), ray, freq);
            return true;
        }
        return false;
    }

    std::ostream &operator<<(std::ostream &os, const Sphere &sp)
    {
      return os << "Sphere origin: " << sp.getOrigin()
                << ", radius: " << sp.getRadius() << " [m]";
    }

    // GETTERS AND SETTERS
    void Sphere::setRadius(float rad)
    {
        radius_ = rad;
    }
    float Sphere::getRadius() const
    {
        return radius_;
    }


    std::ostream &operator<<(std::ostream &os, const SphereWall &sp)
    {
      return os << "SphereWall origin: " << sp.getOrigin()
                << ", radius: " << sp.getRadius() << " [m]";
    }

    // OPERATORS
    EnergyCollector &EnergyCollector::operator=(const EnergyCollector &other)
    {
        if (other == *this)
        {
            return *this;
        }

        this->setOrigin(other.getOrigin());
        this->setRadius(other.getRadius());
        energy_ = other.getEnergy();

        return *this;
    }

    std::ostream &operator<<(std::ostream &os, const EnergyCollector &collector)
    {
      return os << "Energy Collector. Origin: " << collector.getOrigin()
                << ", Radius: " << collector.getRadius();
    }

    bool operator==(const EnergyCollector &left, const EnergyCollector &right)
    {
      return (left.getOrigin() == right.getOrigin() &&
              left.getRadius() == right.getRadius() &&
              left.getEnergy() == right.getEnergy());
    }

    // METHODS

    float EnergyCollector::distanceAt(const core::Vec3 &positionHit) const
    {
        return (getOrigin() - positionHit).magnitude();
    }

    void EnergyCollector::collectEnergy(const core::RayHitData &hitdata)
    {
        // TODO: Energy distribution between many collectors
        energy_ += hitdata.energy();
    }

    // GETTERS AND SETTERS
    void EnergyCollector::setEnergy(float en)
    {
        energy_ = en;
    }
    float EnergyCollector::getEnergy() const
    {
        return energy_;
    }
    void EnergyCollector::addEnergy(float en)
    {
        energy_ += en;
    }

    // CONSTRUCTORS

    TriangleObj::TriangleObj(const core::Vec3 &point1, const core::Vec3 &point2,
                             const core::Vec3 &point3)
        : point1_(point1), point2_(point2), point3_(point3) {
      this->arePointsValid();
      this->refreshAttributes();
    }

    TriangleObj::TriangleObj(const TriangleObj &other)
    {
        this->setPoint1(other.point1());
        this->setPoint2(other.point2());
        this->setPoint3(other.point3());
        this->refreshAttributes();
    }

    // OPERATORS

    TriangleObj &TriangleObj::operator=(const TriangleObj &other)
    {
        this->setPoint1(other.point1());
        this->setPoint2(other.point2());
        this->setPoint3(other.point3());

        this->refreshAttributes();

        return *this;
    }

    bool operator==(const TriangleObj &left, const TriangleObj &right)
    {
      // if other triangle has the same points but declared in different order,
      // they will be still equal.
      std::vector<core::Vec3> vertexVec(
          {left.point1(), left.point2(), left.point3()});
      std::vector<core::Vec3> refVec(
          {right.point1(), right.point2(), right.point3()});

      for (size_t ind = 0; ind < vertexVec.size(); ++ind) {
        if (std::find(vertexVec.begin(), vertexVec.end(), refVec.at(ind)) ==
            vertexVec.end()) {
          return false;
        }
      }
        return true;
    }

    bool operator!=(const TriangleObj &left, const TriangleObj &right)
    {
        return (!(left == right));
    }

    std::ostream &operator<<(std::ostream &os, const TriangleObj &object)
    {
      return os << "Triangle Object, vertex: " << object.point1() << ", "
                << object.point2() << ", " << object.point3();
    }

    // METHODS

    core::Vec3 TriangleObj::normal(const core::Vec3 &surfacePoint) const
    {
        return normal_;
    }

    bool TriangleObj::hitObject(const core::Ray &ray, float freq, core::RayHitData *hitData)
    {
        // if ray direction is parpedicular to normal, there is no hit. It can be translated into
        // checking if scalarProduct of the ray.direction and normal is close or equal to zero.
        float parpCoeff = ray.getDirection().scalarProduct(normal_);
        if (std::abs(parpCoeff) <= constants::kAccuracy)
        {
            return false;
        }

        // Following code calculates time at which ray
        // is hitting surface where triangle is positioned
        float time = (-1 * (ray.getOrigin() - point3_)).scalarProduct(normal_) / parpCoeff;

        // Following code is making sure that ray doesn't hit the same object.
        if (time < constants::kHitAccuracy)
        {
            return false;
        }

        core::Vec3 surfaceHit = ray.at(time);

        if (doesHit(surfaceHit))
        {
            *hitData = core::RayHitData(time, normal_, ray, freq);
            return true;
        }
        return false;
    }

    bool TriangleObj::doesHit(const core::Vec3 &point) const
    {
        core::Vec3 vecA = point1_ - point;
        core::Vec3 vecB = point2_ - point;
        core::Vec3 vecC = point3_ - point;

        //  Area of the triangle made with point and w triangle points.
        float alpha = vecB.crossProduct(vecC).magnitude() / 2;
        float beta = vecC.crossProduct(vecA).magnitude() / 2;
        float gamma = vecA.crossProduct(vecB).magnitude() / 2;

        return (((alpha + beta + gamma) > area_ + constants::kHitAccuracy) ? false : true);
    }

    float TriangleObj::area() const
    {
        return area_;
    }

    void TriangleObj::refreshAttributes()
    {
        this->recalculateArea();
        this->recalculateNormal();
        this->setOrigin((point1_ + point2_ + point3_) / 3);
    }

    // PRIVATE METHODS
    void TriangleObj::recalculateArea()
    {
        core::Vec3 vecA = point1_ - point2_;
        core::Vec3 vecB = point1_ - point3_;
        area_ = vecA.crossProduct(vecB).magnitude() / 2;
    }

    void TriangleObj::recalculateNormal()
    {
        core::Vec3 vecA = point1_ - point2_;
        core::Vec3 vecB = point1_ - point3_;
        core::Vec3 perpendicular = vecA.crossProduct(vecB);
        normal_ = perpendicular.normalize();
    }

    bool TriangleObj::arePointsValid()
    {

      // TODO: Repace this condition to area() < constants::kAccuracy
      // it required a lot of changes how constructors of objects are implemented
      if (point1_ == point2_ || point1_ == point3_ || point2_ == point3_) {
        std::stringstream ss;
        ss << "one point is duplicate of another \n"
           << "point 1: " << point1_ << "\n"
           << "point 2: " << point2_ << "\n"
           << "point 3: " << point3_;
        throw std::invalid_argument(ss.str());
        } else if (area() < constants::kAccuracy) { // THIS auto formating
                                                    // sucks, i dont like it
          std::stringstream ss;
          ss << "area of triangle is too small: " << area();
          throw std::invalid_argument(ss.str());
        }

        core::Vec3 alpha = point1_ - point2_;
        core::Vec3 beta = point1_ - point3_;

        if (alpha.crossProduct(beta) == core::Vec3(0, 0, 0))
        {
          std::stringstream ss;
          ss << "points of the triangle cannot be at the same line \n"
             << "point 1: " << point1_ << "\n"
             << "point 2: " << point2_ << "\n"
             << "point 3: " << point3_;
          throw std::invalid_argument(ss.str());
        }
        return true;
    }

    // GETTERS AND SETTERS
    core::Vec3 TriangleObj::point1() const
    {
        return point1_;
    }
    void TriangleObj::setPoint1(const core::Vec3 &point)
    {
        this->point1_ = point;
    }

    core::Vec3 TriangleObj::point2() const
    {
        return point2_;
    }
    void TriangleObj::setPoint2(const core::Vec3 &point)
    {
        this->point2_ = point;
    }

    core::Vec3 TriangleObj::point3() const
    {
        return point3_;
    }
    void TriangleObj::setPoint3(const core::Vec3 &point)
    {
        this->point3_ = point;
    }

} // namespace objects
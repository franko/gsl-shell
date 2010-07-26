    virtual void apply_transform(agg:trans_affine& m)
    {
      double& x = m_matrix.tx;
      double& y = m_matrix.ty;
      m.transform(&x, &y);
    };
